/*!
 * \file
 * Implements tests for the handling of interleaved timeouts and responses at the lowest level.
 */
#include <riak/error.hxx>
#include <riak/request_with_timeout.hxx>
#include <test/mocks/message.hxx>
#include <test/mocks/transport.hxx>
#include <test/mocks/utility/timer.hxx>
#include <test/mocks/utility/timer_factory.hxx>

//=============================================================================
namespace riak {
	namespace test {
//=============================================================================

using namespace ::testing;

//=============================================================================
		namespace {
//=============================================================================

bool is_ge_when_added_to (const std::size_t delta, std::size_t& target, const std::size_t threshold) {
	target += delta;
	return target >= threshold;
}

bool is_ge_when_added_to_handler (const std::error_code&, const std::size_t bytes_received, const std::string&, std::size_t& target, const std::size_t threshold) {
	// People need to move to >= g++4.6 already -- this was a perfect lambda.
	//
	return is_ge_when_added_to(bytes_received, target, threshold);
}

//=============================================================================
		}   // namespace (anonymous)
//=============================================================================

TEST(request_timeout_handling, when_response_is_first_timeout_is_ignored) {
	/*
	 * TODO: I sense an opportunity for reusable pieces that will make these tests
	 * more terse and thus more useful to the reader.
	 */
	// ========= 
	NiceMock<mock::transport::device> network;
	NiceMock<mock::transport::device::option_to_terminate_request> closure_signal;
	riak::transport::response_handler reply_from_server;
	ON_CALL(network, deliver(_, _)).WillByDefault(
		DoAll(
			SaveArg<1>(&reply_from_server),
			Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))
		));

	std::unique_ptr<mock::utility::timer> the_request_timer(new NiceMock<mock::utility::timer>);
	::riak::utility::timer::callback the_timer_callback;
	ON_CALL(*the_request_timer, run_on_timeout(_, _)).WillByDefault(SaveArg<1>(&the_timer_callback));

	NiceMock<mock::utility::timer_factory> timer_factory;
	ON_CALL(timer_factory, __create()).WillByDefault(Return(the_request_timer.release()));
	// =========

	mock::message::handler response_handler;
	auto the_request = std::make_shared<request_with_timeout>(
			"some data",
			std::chrono::milliseconds(5000),
			response_handler.as_message_handler(),
			timer_factory);
	the_request->dispatch_via(network.as_delivery_provider());

	const std::size_t sent = 10;
	std::size_t received = 0;

	using namespace std::placeholders;
	EXPECT_CALL(response_handler, handle(std::error_code(), Gt(0u), _))
		.Times(AtLeast(1))
		.WillRepeatedly(Invoke(std::bind(is_ge_when_added_to_handler, _1, _2, _3, std::ref(received), sent)));

	reply_from_server(std::error_code(), sent, "some response");
	the_timer_callback(std::error_code());
}


TEST(request_timeout_handling, when_timeout_is_first_response_is_ignored) {
	/*
	 * TODO: I sense an opportunity for reusable pieces that will make these tests
	 * more terse and thus more useful to the reader.
	 */
	// ========= 
	NiceMock<mock::transport::device> network;
	NiceMock<mock::transport::device::option_to_terminate_request> closure_signal;
	riak::transport::response_handler reply_from_server;
	ON_CALL(network, deliver(_, _)).WillByDefault(
		DoAll(
			SaveArg<1>(&reply_from_server),
			Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))
		));

	std::unique_ptr<mock::utility::timer> the_request_timer(new NiceMock<mock::utility::timer>);
	::riak::utility::timer::callback the_timer_callback;
	ON_CALL(*the_request_timer, run_on_timeout(_, _)).WillByDefault(SaveArg<1>(&the_timer_callback));

	NiceMock<mock::utility::timer_factory> timer_factory;
	ON_CALL(timer_factory, __create()).WillByDefault(Return(the_request_timer.release()));
	// =========

	mock::message::handler response_handler;
	auto the_request = std::make_shared<request_with_timeout>(
			"some data",
			std::chrono::milliseconds(5000),
			response_handler.as_message_handler(),
			timer_factory);
	the_request->dispatch_via(network.as_delivery_provider());

	const auto timeout = riak::make_error_code(communication_failure::response_timeout);
	EXPECT_CALL(response_handler, handle(timeout, 0, _))
		.Times(1)
		.WillOnce(Return(true));

	the_timer_callback(std::error_code());
	reply_from_server(std::error_code(), 10, "some response");
}


TEST(request_timeout_handling, timeout_is_reported_properly) {
	/*
	 * TODO: I sense an opportunity for reusable pieces that will make these tests
	 * more terse and thus more useful to the reader.
	 */
	// ========= 
	NiceMock<mock::transport::device> network;
	NiceMock<mock::transport::device::option_to_terminate_request> closure_signal;
	riak::transport::response_handler reply_from_server;
	ON_CALL(network, deliver(_, _)).WillByDefault(
		DoAll(
			SaveArg<1>(&reply_from_server),
			Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))
		));

	std::unique_ptr<mock::utility::timer> the_request_timer(new NiceMock<mock::utility::timer>);
	::riak::utility::timer::callback the_timer_callback;
	ON_CALL(*the_request_timer, run_on_timeout(_, _)).WillByDefault(SaveArg<1>(&the_timer_callback));

	NiceMock<mock::utility::timer_factory> timer_factory;
	ON_CALL(timer_factory, __create()).WillByDefault(Return(the_request_timer.release()));
	// =========

	mock::message::handler response_handler;
	auto the_request = std::make_shared<request_with_timeout>(
			"some data",
			std::chrono::milliseconds(5000),
			response_handler.as_message_handler(),
			timer_factory);
	the_request->dispatch_via(network.as_delivery_provider());

	const auto timeout = riak::make_error_code(communication_failure::response_timeout);
	EXPECT_CALL(response_handler, handle(timeout, 0, _))
		.Times(1)
		.WillOnce(Return(true));

	the_timer_callback(std::error_code());
}


TEST(request_timeout_handling, response_is_reported_properly) {
	/*
	 * TODO: I sense an opportunity for reusable pieces that will make these tests
	 * more terse and thus more useful to the reader.
	 */
	// ========= 
	NiceMock<mock::transport::device> network;
	NiceMock<mock::transport::device::option_to_terminate_request> closure_signal;
	riak::transport::response_handler reply_from_server;
	ON_CALL(network, deliver(_, _)).WillByDefault(
		DoAll(
			SaveArg<1>(&reply_from_server),
			Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))
		));

	std::unique_ptr<mock::utility::timer> the_request_timer(new NiceMock<mock::utility::timer>);
	::riak::utility::timer::callback the_timer_callback;
	ON_CALL(*the_request_timer, run_on_timeout(_, _)).WillByDefault(SaveArg<1>(&the_timer_callback));

	NiceMock<mock::utility::timer_factory> timer_factory;
	ON_CALL(timer_factory, __create()).WillByDefault(Return(the_request_timer.release()));
	// =========

	mock::message::handler response_handler;
	auto the_request = std::make_shared<request_with_timeout>(
			"some data",
			std::chrono::milliseconds(5000),
			response_handler.as_message_handler(),
			timer_factory);
	the_request->dispatch_via(network.as_delivery_provider());

	using namespace std::placeholders;
	const std::size_t sent = 10;
	std::size_t received = 0;
	EXPECT_CALL(response_handler, handle(std::error_code(), Gt(0u), _))
		.Times(AtLeast(1))
		.WillRepeatedly(Invoke(std::bind(is_ge_when_added_to_handler, _1, _2, _3, std::ref(received), sent)));

	reply_from_server(std::error_code(), sent, "some response");
}

//=============================================================================
	}   // namespace test
}   // namespace riak
//=============================================================================
