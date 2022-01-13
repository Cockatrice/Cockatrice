// max sizes of strings used in the protocol

// max size for short strings, like names and things that are generally a single phrase
constexpr int MAX_NAME_LENGTH = 0xff;
// max size for chat messages and text contents
constexpr int MAX_TEXT_LENGTH = 0xfff;
// max size for deck files and pictures
constexpr int MAX_FILE_LENGTH = 0xfffff; // about a megabyte
