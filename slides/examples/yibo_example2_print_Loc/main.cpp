#include <format>
#include <iostream>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace ct = clang::tooling;
using namespace std::literals;

static llvm::cl::OptionCategory toolCategory("Tool Options");
static llvm::cl::opt<bool> onlyMainFile(
    "main-file-only",
    llvm::cl::desc("Only process includes from the main file"),
    llvm::cl::init(false),
    llvm::cl::cat(toolCategory)
);

std::string locationToString(const clang::SourceManager &sourceManager,
                             clang::SourceLocation sourceLoc) {
    return std::format("{}:{}:{}",
                       std::string(sourceManager.getFilename(sourceLoc)),
                       sourceManager.getSpellingLineNumber(sourceLoc),
                       sourceManager.getSpellingColumnNumber(sourceLoc));
}

class FindIncludes : public clang::PPCallbacks {
public:
    FindIncludes(clang::SourceManager &sourceManager) : sourceManager_(&sourceManager) {}

    void InclusionDirective(clang::SourceLocation hashLoc,
                            const clang::Token &, llvm::StringRef fileName,
                            bool isAngled, clang::CharSourceRange,
                            clang::OptionalFileEntryRef file,
                            llvm::StringRef, llvm::StringRef,
                            const clang::Module *,
                            clang::SrcMgr::CharacteristicKind) override {
        // Check if we should ignore includes outside the main file
        if (onlyMainFile && !sourceManager_->isInMainFile(hashLoc)) {
            return;
        }

        std::string actualFileName;
        if (file) { // If the file is found, get its name这个是由点奇怪，这个file获得的前提是commandline database是存在的。但locationToString(*sourceManager_, hashLoc)却不需要。
            actualFileName = file->getName();
        }

        std::string headerName = isAngled ? ("<"s + std::string(fileName) + ">"s)
                                          : ("\""s + std::string(fileName) + "\""s);

        llvm::outs() << std::format("include directive:\n    location: {}\n"
                                    "    header: {}\n    pathname: {}\n",
                                    locationToString(*sourceManager_, hashLoc),
                                    headerName, actualFileName);
    }

private:
    clang::SourceManager *sourceManager_;
};

class IncludeFinderAction : public clang::PreprocessOnlyAction {
    bool BeginSourceFileAction(clang::CompilerInstance &ci) override {
        std::unique_ptr<FindIncludes> findIncludes(new FindIncludes(ci.getSourceManager()));
        clang::Preprocessor &pp = ci.getPreprocessor();
        pp.SetSuppressIncludeNotFoundError(true);
        pp.addPPCallbacks(std::move(findIncludes));
        return true;
    }
};

int main(int argc, char **argv) {
    auto expectedOptionsParser = ct::CommonOptionsParser::create(argc, const_cast<const char**>(argv), toolCategory);
    if (!expectedOptionsParser) {
        llvm::errs() << llvm::toString(expectedOptionsParser.takeError());
        return 1;
    }

    ct::CommonOptionsParser &optionsParser = *expectedOptionsParser;
    ct::ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());
    return tool.run(ct::newFrontendActionFactory<IncludeFinderAction>().get());
}
