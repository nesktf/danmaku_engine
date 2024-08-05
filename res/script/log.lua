local _M = {}

function _M.error(format_str, ...)
  __LOG_ERROR(string.format(format_str, ...))
end

function _M.warning(format_str, ...)
  __LOG_WARNING(string.format(format_str, ...))
end

function _M.debug(format_str, ...)
  __LOG_DEBUG(string.format(format_str, ...))
end

function _M.info(format_str, ...)
  __LOG_INFO(string.format(format_str, ...))
end

function _M.verbose(format_str, ...)
  __LOG_VERBOSE(string.format(format_str, ...))
end

return _M
