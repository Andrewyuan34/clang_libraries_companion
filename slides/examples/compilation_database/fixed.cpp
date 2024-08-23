#include <format>
#include <utility>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/Config/llvm-config.h>
#include "utility.hpp"

namespace ct = clang::tooling;

int main(int argc, char** argv) {
	if (argc < 2) {
		llvm::errs() << "no fixed database specified\n";
		return 1;
	}
	std::string pathname(argv[1]);
	std::string errString;
	std::unique_ptr<ct::CompilationDatabase> compDatabase;
	compDatabase = ct::FixedCompilationDatabase::loadFromFile(pathname, 
	  errString); // first parameter is the path to the fixed database, second parameter is the error string, which will be filled in case of an error
	if (!compDatabase) {
		llvm::errs() << std::format("ERROR: {}\n", errString);
		return 1;
	}

	// get all files in the database
	std::vector<std::string> sourcePaths = compDatabase->getAllFiles();
	// print the number of files in the database. and for fixed databases, this number is always 0
	llvm::outs() << std::format("Number of files: {}\n", sourcePaths.size()); 
	for (const auto& sourcePath : sourcePaths) { // So this loop actually does not print anything
		llvm::outs() << std::format("{}\n", sourcePath);
	}

	// get all compile commands in the database, but for fixed databases, this number is always 0
	std::vector<ct::CompileCommand> compCommands =
	  compDatabase->getAllCompileCommands();
	llvm::outs() << std::format("Number of compile commands: {}\n", 
	  compCommands.size());
	printCompCommands(llvm::outs(), compCommands);

	// get compile commands for the files specified in the command line
	for (int i = 2; i < argc; ++i) {
		std::vector<ct::CompileCommand> compCommands =
		  compDatabase->getCompileCommands(argv[i]); //getCompileCommands (StringRef FilePath) returns a vector of CompileCommand objects for the specified file
		printCompCommands(llvm::outs(), compCommands);
	}
	return 0;
}
