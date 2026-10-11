@page logging Logging

Cockatrice uses QtLogging from the QtCore module for its logging. See
the [official documentation](https://doc.qt.io/qt-6/qtlogging.html) for further details.

# Log Message Pattern

Any message logged through the QtLogging system automatically conforms to this message pattern:

Generic:

```
[<timestamp> <log_level>] [<class:function>] - <message> [<filename>:<line_no>]
```

Example:

```
[2025-12-05 14:48:25.908 I] [MainWindow::startupConfigCheck] - Startup: found config with current version [window_main.cpp:951]
```

For more information, see [Logging Setup](#logging-setup).

# Log Level and Categories

\note The default log level for the application is info.

This means that you should only use qInfo() in production-level code if you are truly sure that this message is
beneficial to end-users and other developers. As a general rule, if your functionality logs to info more than twice in
response to a user interaction, you are advised to consider moving some of these logs down to the debug level.

\warning You are strongly advised to avoid the use of the generic logging macros (e.g. qDebug(), qInfo(), qWarn()).

\note You should instead use the corresponding category logging macros (qCDebug(), qCInfo(), qCWarn()) and define
logging
categories for your log statements.

Example:

```c++
in .h

inline Q_LOGGING_CATEGORY(ExampleCategory, "cockatrice_example_category");
inline Q_LOGGING_CATEGORY(ExampleSubCategory, "cockatrice_example_category.sub_category");

in .cpp

qCInfo(ExampleCategory) << "Info level logs are usually sent through the main category"
qCDebug(ExampleSubCategory) << "Debug level logs are permitted their own category to allow selective silencing"
```

For more information on how to enable or disable logging categories,
see [Logging Configuration](#logging-configuration).

# Logging Configuration

For configuring our logging, we use the qtlogging.ini, located under cockatrice/resources/config/qtlogging.ini, which is
baked into the application in release version and set as the QT_LOGGING_CONF environment variable in main.cpp.

```c++
#ifdef Q_OS_APPLE
    // <build>/cockatrice/cockatrice.app/Contents/MacOS/cockatrice
    const QByteArray configPath = "../../../qtlogging.ini";
#elif defined(Q_OS_UNIX)
    // <build>/cockatrice/cockatrice
    const QByteArray configPath = "./qtlogging.ini";
#elif defined(Q_OS_WIN)
    // <build>/cockatrice/Debug/cockatrice.exe
    const QByteArray configPath = "../qtlogging.ini";
#else
    const QByteArray configPath = "";
#endif

    if (!qEnvironmentVariableIsSet(("QT_LOGGING_CONF"))) {
        // Set the QT_LOGGING_CONF environment variable
        qputenv("QT_LOGGING_CONF", configPath);
    }
```

For more information on how to use this file and on how Qt evaluates which logging rules/file to use, please
see the [official Qt documentation](https://doc.qt.io/qt-6/qloggingcategory.html#configuring-categories).

Some examples:

```
# Turn off all logging except everything from card_picture_loader and all sub categories

[Rules]
# The default log level is info
*.debug = false
*.info = false
*.warning = false
*.critical = false
*.fatal = false

card_picture_loader.* = true
```

```
# Turn off all logging except info level logs from card_picture_loader and all sub categories

[Rules]
# The default log level is info
*.debug = false
*.info = false
*.warning = false
*.critical = false
*.fatal = false

card_picture_loader.*.info = true
```

```
[Rules]
# Turn on debug level logs for card_picture_loader but keep logging for sub categories suppressed
*.debug = false

card_picture_loader.debug = true
```

```
[Rules]
# Turn on all logs for worker subcategory of card_picture_loader
*.debug = false

card_picture_loader.worker = true
```

```
[Rules]
# Turn off some noisy and irrelevant startup logging for local development
*.debug = false

qt_translator = false
window_main.* = false
release_channel = false
spoiler_background_updater = false
theme_manager = false
sound_engine = false
tapped_out_interface = false
card_database = false
card_database.loading = false
card_database.loading.success_or_failure = true
cockatrice_xml.* = false
```

# Logging Setup

This is achieved through our logging setup in @ref main.cpp, where we set the message pattern and install a custom
logger which replaces the full file path at the end with just the file name (Qt shows the full and quite lengthy path by
default).

```c++
    qSetMessagePattern(
        "\033[0m[%{time yyyy-MM-dd h:mm:ss.zzz} "
        "%{if-debug}\033[36mD%{endif}%{if-info}\033[32mI%{endif}%{if-warning}\033[33mW%{endif}%{if-critical}\033[31mC%{"
        "endif}%{if-fatal}\033[1;31mF%{endif}\033[0m] [%{function}] - %{message} [%{file}:%{line}]");
    QApplication app(argc, argv);

    QObject::connect(&app, &QApplication::lastWindowClosed, &app, &QApplication::quit);

    qInstallMessageHandler(CockatriceLogger);
```

```c++
static void CockatriceLogger(QtMsgType type, const QMessageLogContext &ctx, const QString &message)
{
    QString logMessage = qFormatLogMessage(type, ctx, message);

    // Regular expression to match the full path in the square brackets and extract only the filename and line number
    QRegularExpression regex(R"(\[(?:.:)?[\/\\].*[\/\\]([^\/\\]+\:\d+)\])");
    QRegularExpressionMatch match = regex.match(logMessage);

    if (match.hasMatch()) {
        // Extract the filename and line number (e.g., "main.cpp:211")
        QString filenameLine = match.captured(1);

        // Replace the full path in square brackets with just the filename and line number
        logMessage.replace(match.captured(0), QString("[%1]").arg(filenameLine));
    }

    Logger::getInstance().log(type, ctx, logMessage);
}
```