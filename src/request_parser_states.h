#ifndef REQUEST_PARSER_STATES_H
# define REQUEST_PARSER_STATES_H

# include <stdexcept>

class Request;

class RequestState {
public:
	// virtual ~RequestState();
	virtual void clear(Request* request) = 0;
	virtual void clear_request(Request* request) = 0;
	virtual void parse(Request* request) = 0;

	class InvalidState : public std::runtime_error {
	public:
		InvalidState(const char* msg);
	};
};

namespace RequestStates {
	class Init : public RequestState {
	public:
		// Singleton access
		static RequestState* get_instance();
		void clear(Request* request);
		void clear_request(Request* request);
		void parse(Request* request);
	private:
		// Empty implemetation to dissmiss
		Init() {};
		// Override defaults to dismiss
		Init(const Init& src);
		Init& operator=(const Init& src);
	};

	class ReadingBuffer : public RequestState {
	public:
		static RequestState* get_instance();
		void clear(Request* request);
		void clear_request(Request* request);
		void parse(Request* request);
	private:
		ReadingBuffer() {};
		ReadingBuffer(const ReadingBuffer& src);
		ReadingBuffer& operator=(const ReadingBuffer& src);
	};

	class ParsingHead : public RequestState {
	public:
		static RequestState* get_instance();
		void clear(Request* request);
		void clear_request(Request* request);
		void parse(Request* request);
	private:
		ParsingHead() {};
		ParsingHead(const ParsingHead& src);
		ParsingHead& operator=(const ParsingHead& src);
	};

	class ExtractingBody : public RequestState {
	public:
		static RequestState* get_instance();
		void clear(Request* request);
		void clear_request(Request* request);
		void parse(Request* request);
	private:
		ExtractingBody() {};
		ExtractingBody(const ExtractingBody& src);
		ExtractingBody& operator=(const ExtractingBody& src);
	};

	class Done : public RequestState {
	public:
		static RequestState* get_instance();
		void clear(Request* request);
		void clear_request(Request* request);
		void parse(Request* request);
	private:
		Done() {};
		Done(const Done& src);
		Done& operator=(const Done& src);
	};

	class Invalid : public RequestState {
	public:
		static RequestState* get_instance();
		void clear(Request* request);
		void clear_request(Request* request);
		void parse(Request* request);
	private:
		Invalid() {};
		Invalid(const Invalid& src);
		Invalid& operator=(const Invalid& src);
	};
}

#endif
