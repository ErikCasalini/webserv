// #include "http_types.h"
#include "request_parser.h"
#include "request_parser_states.h"
#include <string>

using std::string;

RequestState::InvalidState::InvalidState(const char* msg)
	: std::runtime_error(msg)
{};

RequestState* RequestStates::Init::get_instance()
{
	static RequestStates::Init singleton;
	return (&singleton);
};

void RequestStates::Init::clear(Request* request)
{
	if (request->m_buffer.length() > 0) {
		request->set_state(RequestStates::Invalid::get_instance());
		throw RequestState::InvalidState(
			"Can't call clear() in 'Init' state with a non empty buffer");
	}
	request->_clear();
}

void RequestStates::Init::clear_request(Request* request)
{
	request->_clear_infos();
}

void RequestStates::Init::parse(Request* request)
{
	request->set_state(RequestStates::ReadingBuffer::get_instance());
	request->parse();
}

RequestState* RequestStates::ReadingBuffer::get_instance()
{
	static RequestStates::ReadingBuffer singleton;
	return (&singleton);
};

void RequestStates::ReadingBuffer::clear(Request* request)
{
	request->set_state(RequestStates::Invalid::get_instance());
	throw RequestState::InvalidState("Can't call clear() in 'ReadingBuffer' state");
	(void)request;
}

void RequestStates::ReadingBuffer::clear_request(Request* request)
{
	request->set_state(RequestStates::Invalid::get_instance());
	throw RequestState::InvalidState("Can't call clear() in 'ReadingBuffer' state");
	(void)request;
}

void RequestStates::ReadingBuffer::parse(Request* request)
{
// We fill artificially the buffer with a special ctor for testing.
#ifndef TESTING
	ssize_t ret = _Request::read_socket(request->m_sockfd, request->m_buffer, request->m_recv_buf_size);
#else
	ssize_t ret = 1;
#endif
	if (ret == 0) {
		request->set_state(RequestStates::Invalid::get_instance());
		throw Request::ConnectionClosed("socket closed");
	}
	// TODO replace this exception by an io exception (check for Erik's exceptions)
	if (ret == -1) {
		request->set_state(RequestStates::Invalid::get_instance());
		throw std::runtime_error("recv failed");
	}
	// Wait for the two consecutive spaces to actually parse the input
	if (request->m_infos.method == post && request->m_infos.headers.content_length > 0) {
		request->set_state(RequestStates::ExtractingBody::get_instance());
		request->parse();
		return ;
	} else if (request->m_buffer.find(CRLF CRLF) != string::npos) {
		request->set_state(RequestStates::ParsingHead::get_instance());
		request->parse();
	}
}

RequestState* RequestStates::ParsingHead::get_instance()
{
	static RequestStates::ParsingHead singleton;
	return (&singleton);
};

void RequestStates::ParsingHead::clear(Request* request)
{
	request->set_state(RequestStates::Invalid::get_instance());
	throw RequestState::InvalidState("Can't call clear() in 'ParsingHead' state");
	(void)request;
}

void RequestStates::ParsingHead::clear_request(Request* request)
{
	request->set_state(RequestStates::Invalid::get_instance());
	throw RequestState::InvalidState("Can't call clear() in 'ParsingHead' state");
	(void)request;
}

void RequestStates::ParsingHead::parse(Request* request)
{
	// More direct access to members of request.
	string& buffer = request->m_buffer;
	size_t& pos = request->m_pos;
	request_t& infos = request->m_infos;

	// The rfc9112 specify that we SHOULD ignore at least one crlf prior to the request.
	while (buffer.substr(pos, 2) == CRLF)
		pos += 2;
	
	try {
		// request line
		infos.method = _Request::parse_method(buffer, pos);
		_Request::consume_sp(buffer, pos);
		infos.target = _Request::parse_target(buffer, pos);
		// Start line for 0.9 don't have the PROTOCOL field
		if (buffer.substr(pos, 2) == CRLF) {
			infos.protocol = zero_nine;
			throw Request::NotImplemented("HTTP 0.9 is not handled");
		}
		_Request::consume_sp(buffer, pos);
		infos.protocol = _Request::parse_protocol(buffer, pos);
		_Request::consume_crlf(buffer, pos);
		
		// headers
		infos.headers = _Request::parse_headers(buffer, pos, infos);
		_Request::consume_crlf(buffer, pos);
		if (infos.headers.content_length > 0) {
			request->set_state(RequestStates::ExtractingBody::get_instance());
			request->parse();
		} else {
			infos.status = ok;
			request->set_state(RequestStates::Done::get_instance());
			request->erase_parsed();
		}
	} catch (const Request::BadRequest& e) {
		infos.status = bad_request;
		request->set_state(RequestStates::Done::get_instance());
		request->erase_parsed();
	} catch (const Request::NotImplemented& e) {
		infos.status = not_implemented;
		request->set_state(RequestStates::Done::get_instance());
		request->erase_parsed();
	}
}

RequestState* RequestStates::ExtractingBody::get_instance()
{
	static RequestStates::ExtractingBody singleton;
	return (&singleton);
};

void RequestStates::ExtractingBody::clear(Request* request)
{
	request->set_state(RequestStates::Invalid::get_instance());
	throw RequestState::InvalidState("Can't call clear() in 'ExtractingBody' state");
	(void)request;
}

void RequestStates::ExtractingBody::clear_request(Request* request)
{
	request->set_state(RequestStates::Invalid::get_instance());
	throw RequestState::InvalidState("Can't call clear() in 'ExtractingBody' state");
	(void)request;
}

void RequestStates::ExtractingBody::parse(Request* request)
{
	request->m_infos.body.append(_Request::extract_body(request->m_buffer,
														request->m_pos,
														request->m_infos));
	if (request->m_infos.status == parsing) {
		request->set_state(RequestStates::ReadingBuffer::get_instance());
	} else {
		request->set_state(RequestStates::Done::get_instance());
		request->erase_parsed();
	}
}

RequestState* RequestStates::Done::get_instance()
{
	static RequestStates::Done singleton;
	return (&singleton);
};

void RequestStates::Done::clear(Request* request)
{
	if (request->m_buffer.length() > 0) {
		request->set_state(RequestStates::Invalid::get_instance());
		throw RequestState::InvalidState(
			"Can't call clear() in 'Done' state with a non empty buffer");
	}
	request->_clear();
	request->set_state(RequestStates::Init::get_instance());
}

void RequestStates::Done::clear_request(Request* request)
{
	request->_clear_infos();
	request->set_state(RequestStates::Init::get_instance());
}

void RequestStates::Done::parse(Request* request)
{
	request->set_state(RequestStates::Invalid::get_instance());
	throw RequestState::InvalidState("Can't call parse() in 'Done' state");
	(void)request;
}

RequestState* RequestStates::Invalid::get_instance()
{
	static RequestStates::Invalid singleton;
	return (&singleton);
};

void RequestStates::Invalid::clear(Request* request)
{
	request->_clear();
	request->set_state(RequestStates::Init::get_instance());
}

void RequestStates::Invalid::clear_request(Request* request)
{
	throw RequestState::InvalidState("Must call clear() in 'Invalid' state");
	(void)request;
}

void RequestStates::Invalid::parse(Request* request)
{
	throw RequestState::InvalidState("Can't call parse() in 'Invalid' state");
	(void)request;
}
