/**
 * @file
 */

#include "Input.h"
#include "core/Log.h"
#include "core/command/CommandHandler.h"
#include <string.h>

namespace core {

bool Input::init(uv_loop_t* loop) {
	_tty.data = this;
	uv_tty_init(loop, &_tty, 0, 1);

	uv_stream_t* stream = reinterpret_cast<uv_stream_t*>(&_tty);
	if (uv_is_readable(stream)) {
		uv_read_start(stream, onAllocBuffer, onRead);
	}

	return true;
}

void Input::shutdown() {
}

void Input::onAllocBuffer(uv_handle_t *handle, size_t suggestedSize, uv_buf_t *buf) {
	Input* self = static_cast<Input*>(handle->data);
	buf->len = sizeof(self->_input);
	buf->base = self->_input;
}

void Input::onRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
	if (nread < 0) {
		return uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);
	}
	size_t len;
	for (len = 0u; len < buf->len; ++len) {
		if (buf->base[len] == '\0' || buf->base[len] == '\n' || buf->base[len] == '\r') {
			break;
		}
	}
	if (len == 0u || len == buf->len) {
		return;
	}
	const std::string commandLine(buf->base, len);
	core::executeCommands(commandLine);
}

}
