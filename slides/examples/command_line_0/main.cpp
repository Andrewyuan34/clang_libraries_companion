/*
#include <format>
#include <string>
#include <string_view>
#include <llvm/Support/CommandLine.h>

namespace lc = llvm::cl;

static lc::OptionCategory toolOpts("Tool options");
static lc::opt<std::string> logFile("log-file", lc::ZeroOrMore);
static lc::opt<bool> verbose("verbose", lc::Optional, lc::cat(toolOpts));
static lc::opt<int> debugLevel("debug-level", lc::Optional,
  lc::init(-1), lc::cat(toolOpts));
static lc::list<int> values("value", lc::ZeroOrMore, lc::cat(toolOpts));
static lc::alias valuesAlias("v", lc::aliasopt(values));
static lc::list<std::string> files(lc::Positional, lc::ZeroOrMore,
  lc::cat(toolOpts));

static void printVersion(llvm::raw_ostream& out) {out << "version 0.0.0\n";}

int main(int argc, char **argv) {
	lc::SetVersionPrinter(printVersion);
	lc::ParseCommandLineOptions(argc, argv,
	  "This program illustrates the use of the LLVM CommandLine API.");
	llvm::outs()
	  << std::format("log file: {}\n",
	  !logFile.empty() ? std::string_view(logFile) : "---")
	  << std::format("debug level: {}\n", static_cast<bool>(debugLevel))
	  << std::format("verbose: {}\n", static_cast<bool>(verbose));
	for (auto i : values) {llvm::outs() << std::format("value: {}\n", i);}
	for (auto i : files) {llvm::outs() << std::format("file: {}\n", i);}
}
*/
#include <format>
#include <string>
#include <string_view>
#include <llvm/Support/CommandLine.h>

namespace lc = llvm::cl;

static lc::OptionCategory toolOpts("Tool options");

// Command line options
// first argument represents the option name, which is used to access the value
// second argument represents the number of arguments, which can be Optional, Required, or ZeroOrMore
// third argument refers to the description of the option, which is displayed in the help message, desc means description
static lc::opt<std::string> logFile("log-file", lc::ZeroOrMore, lc::desc("Specify log file"));
static lc::opt<bool> verbose("verbose", lc::Optional, lc::desc("Enable verbose output"), lc::cat(toolOpts));
static lc::opt<int> debugLevel("debug-level", lc::Optional, lc::init(-1), lc::desc("Set the debug level"), lc::cat(toolOpts));
static lc::list<int> values("value", lc::ZeroOrMore, lc::desc("Specify values"), lc::cat(toolOpts));
static lc::alias valuesAlias("v", lc::aliasopt(values), lc::desc("Alias for 'value'"));
static lc::list<std::string> files(lc::Positional, lc::ZeroOrMore, lc::desc("Specify input files"), lc::cat(toolOpts));

// Additional options
static lc::opt<std::string> outputFile("output-file", lc::desc("Specify output file"), lc::cat(toolOpts));
static lc::opt<int> count("count", lc::Optional, lc::init(1), lc::desc("Number of iterations"), lc::cat(toolOpts));
static lc::opt<bool> flag("flag", lc::Optional, lc::desc("A boolean flag"), lc::cat(toolOpts));

// Function to print the version
static void printVersion(llvm::raw_ostream& out) {
    out << "version 1.0.0\n";
}

// Function to handle errors
static void handleError(const std::string& errorMsg) {
    llvm::errs() << "Error: " << errorMsg << "\n";
    llvm::errs() << "Use --help to see available options.\n";
}

int main(int argc, char **argv) {
    // Set the version printer and parse command line options
    lc::SetVersionPrinter(printVersion);
    lc::ParseCommandLineOptions(argc, argv,
      "This program illustrates the use of the LLVM CommandLine API.");

    // Check for invalid options or missing required options
    if (values.empty() && !files.empty()) {
        handleError("At least one 'value' must be specified if files are provided.");
        return 1;
    }

    // Print parsed options
    llvm::outs()
      << std::format("log file: {}\n", !logFile.empty() ? std::string_view(logFile) : "---")
      << std::format("debug level: {}\n", debugLevel.getValue())
      << std::format("verbose: {}\n", verbose ? "enabled" : "disabled")
      << std::format("output file: {}\n", !outputFile.empty() ? std::string_view(outputFile) : "not specified")
      << std::format("count: {}\n", count.getValue())
      << std::format("flag: {}\n", flag ? "true" : "false");

    // Print values
    for (auto i : values) {
        llvm::outs() << std::format("value: {}\n", i);
    }

    // Print files
    for (auto& file : files) {
        llvm::outs() << std::format("file: {}\n", file);
    }

    return 0;
}
