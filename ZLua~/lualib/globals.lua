function __zluaErrorHandler(err)
    return debug.traceback(err, 2)
end
