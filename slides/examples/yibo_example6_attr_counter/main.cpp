#include <format>
#include <set>
#include <string>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

const std::set<std::string> standardAttributes = {
    "noreturn", "deprecated", "fallthrough", "nodiscard", 
    "maybe_unused", "likely", "unlikely", "carries_dependency",
    "no_unique_address", "assume"
};


struct AttributeCounter {
    std::map<std::string, int> standardAttributeCounts;
    int nonStandardCount = 0;

    void increment(const std::string &attrName) {
        if (standardAttributes.find(attrName) != standardAttributes.end()) {
            standardAttributeCounts[attrName]++;
        } else {
            nonStandardCount++;
        }
    }

    void printResults() const {
        for (const auto &entry : standardAttributeCounts) {
            llvm::outs() << entry.second << " " << entry.first << "\n";
        }
        llvm::outs() << nonStandardCount << " all non-standard attributes (combined)\n";
    }
};


class AttributeMatchCallback : public cam::MatchFinder::MatchCallback {
public:
    AttributeMatchCallback(AttributeCounter &Counter) : Counter(Counter) {}

    virtual void run(const cam::MatchFinder::MatchResult &Result) override {
        if (const clang::Attr *Attr = Result.Nodes.getNodeAs<clang::Attr>("attr")) {
            if(Result.Context->getSourceManager().isWrittenInMainFile(Attr->getLocation())){
                std::string attrName = Attr->getSpelling();
                Counter.increment(attrName);
            }
        }
    }

private:
    AttributeCounter &Counter;
};

static llvm::cl::OptionCategory optionCategory("Tool options");

int main(int argc, const char **argv) {
	auto expectedParser = ct::CommonOptionsParser::create(argc, argv,
	  optionCategory);
	if (!expectedParser) {
		llvm::errs() << llvm::toString(expectedParser.takeError());
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = expectedParser.get();
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());

    AttributeCounter Counter;
    AttributeMatchCallback Callback(Counter);

    cam::MatchFinder Finder;

    // Match attributes in declarations, statements, types, etc.
    Finder.addMatcher(cam::attr().bind("attr"), &Callback);

    tool.run(ct::newFrontendActionFactory(&Finder).get());

    Counter.printResults();
    return 0;
}