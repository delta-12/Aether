#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "log.h"

const char *const LogTag = "TAG";
const char *const LogFormat = "Foo %d %s";
const int LogIntegerArg = -1;
const char *const LogStringArg = "bar";
const char *const LogStringBody = "[TAG] Foo -1 bar\x1B[0m\r\n";

class CustomLogger
{
public:
    virtual void LogTagLevelFormat(const char *const tag, const a_Log_Level_t level, const char *const format) const = 0;
};

class MockLogger : public CustomLogger
{
public:
    MOCK_METHOD(void, LogTagLevelFormat, (const char *const tag, const a_Log_Level_t level, const char *const format), (const, override));
};

class Log : public testing::Test
{
protected:
    static void LogCustom(const char *const tag, const a_Log_Level_t level, const char *const format, ...)
    {
        mock_logger_->LogTagLevelFormat(tag, level, format);
    }

    static std::string GetLogString(const a_Log_Level_t level, const std::string &body)
    {
        std::string prefix;

        switch (level)
        {
        case A_LOG_LEVEL_ERROR:
            prefix = "\x1B[0;31m[ERROR] ";
            break;
        case A_LOG_LEVEL_WARNING:
            prefix = "\x1B[0;33m[WARNING] ";
            break;
        case A_LOG_LEVEL_INFO:
            prefix = "\x1B[0;32m[INFO] ";
            break;
        case A_LOG_LEVEL_DEBUG:
            prefix = "\x1B[0;34m[DEBUG] ";
            break;
        case A_LOG_LEVEL_VERBOSE:
        default:
            prefix = "\x1B[0;35m[VERBOSE] ";
            break;
        }

        return prefix + body;
    }

    void SetUp() override
    {
        mock_logger_ = new MockLogger;
        a_Log_SetLogLevel(A_LOG_LEVEL_VERBOSE);
    }

    void TearDown() override
    {
        delete mock_logger_;
    }

    static MockLogger *mock_logger_;
    std::string expected;
    std::string output;
};

MockLogger *Log::mock_logger_ = nullptr;

TEST_F(Log, SetLogLevel)
{
    a_Log_SetLogLevel(A_LOG_LEVEL_INFO);
    expected = "";
    testing::internal::CaptureStdout();
    A_LOG_DEBUG(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    a_Log_SetLogLevel(A_LOG_LEVEL_MAX);
    expected = "";
    testing::internal::CaptureStdout();
    A_LOG_DEBUG(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    a_Log_SetLogLevel(A_LOG_LEVEL_DEBUG);
    expected = GetLogString(A_LOG_LEVEL_DEBUG, LogStringBody);
    testing::internal::CaptureStdout();
    A_LOG_DEBUG(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);
}

TEST_F(Log, RegisterCustomLogger)
{
    a_Log_RegisterCustomLogger(nullptr);
    expected = GetLogString(A_LOG_LEVEL_INFO, LogStringBody);
    testing::internal::CaptureStdout();
    A_LOG_INFO(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    a_Log_RegisterCustomLogger(LogCustom);
    expected = "";
    testing::internal::CaptureStdout();
    EXPECT_CALL(*mock_logger_, LogTagLevelFormat(testing::StrEq(LogTag), A_LOG_LEVEL_INFO, testing::StrEq(LogFormat))).Times(1);
    A_LOG_INFO(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);
}

TEST_F(Log, Log)
{
    expected = "";
    testing::internal::CaptureStdout();
    a_Log(nullptr, A_LOG_LEVEL_ERROR, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    expected = "";
    testing::internal::CaptureStdout();
    a_Log(LogTag, A_LOG_LEVEL_MAX, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    expected = "";
    testing::internal::CaptureStdout();
    a_Log(LogTag, A_LOG_LEVEL_ERROR, nullptr, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    expected = GetLogString(A_LOG_LEVEL_ERROR, LogStringBody);
    testing::internal::CaptureStdout();
    A_LOG_ERROR(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    expected = GetLogString(A_LOG_LEVEL_WARNING, LogStringBody);
    testing::internal::CaptureStdout();
    A_LOG_WARNING(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    expected = GetLogString(A_LOG_LEVEL_INFO, LogStringBody);
    testing::internal::CaptureStdout();
    A_LOG_INFO(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    expected = GetLogString(A_LOG_LEVEL_DEBUG, LogStringBody);
    testing::internal::CaptureStdout();
    A_LOG_DEBUG(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);

    expected = GetLogString(A_LOG_LEVEL_VERBOSE, LogStringBody);
    testing::internal::CaptureStdout();
    A_LOG_VERBOSE(LogTag, LogFormat, LogIntegerArg, LogStringArg);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(expected, output);
}
