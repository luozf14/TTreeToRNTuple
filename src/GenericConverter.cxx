#include "TTreeToRNTuple.hxx"

#include <string>
#include <vector>

using RException = ROOT::Experimental::RException;

static void Usage(char *progname)
{
    std::cout << "Usage: " << progname << " -i <input.root> -o <output.ntuple> -t(ree) <tree name> "
              << "[-d(ictionary) <dictionary name>] [-s(ub branch) <branch name>]"
              << "[-c(ompression) <compression algorithm>] [-p(rint conversion progress)]"
              << std::endl;
}

int main(int argc, char **argv)
{
    std::string inputFile;
    std::string outputFile;
    std::string treeName;
    std::string compressionAlgo = "none";
    std::vector<std::string> dictionaries = {};
    std::vector<std::string> subBranches = {};
    Bool_t flagDefaultProgressCallbackFunc = false;

    int inputArg;
    while ((inputArg = getopt(argc, argv, "hi:o:c:d:b:t:s:p")) != -1)
    {
        switch (inputArg)
        {
        case 'h':
            Usage(argv[0]);
            return 0;
        case 'i':
            inputFile = optarg;
            break;
        case 'o':
            outputFile = optarg;
            break;
        case 'c':
            compressionAlgo = optarg;
            break;
        case 'd':
            dictionaries.push_back(optarg);
            break;
        case 's':
            subBranches.push_back(optarg);
            break;
        case 't':
            treeName = optarg;
            break;
        case 'p':
            flagDefaultProgressCallbackFunc = true;
            break;
        default:
            fprintf(stderr, "Unknown option: -%c\n", inputArg);
            Usage(argv[0]);
            return 1;
        }
    }

    if (inputFile.empty() || outputFile.empty() || treeName.empty())
    {
        throw RException(R__FAIL("Error: Minimal required parameters: -i <input.root> -o <output.ntuple> -t(ree) <tree name>\n)"));
    }

    std::unique_ptr<TTreeToRNTuple> conversion = std::make_unique<TTreeToRNTuple>(inputFile, outputFile, treeName);
    conversion->SetCompressionAlgo(compressionAlgo);
    conversion->SetDictionary(dictionaries);
    conversion->SelectBranches(subBranches);
    if (flagDefaultProgressCallbackFunc)
        conversion->SetDefaultProgressCallbackFunc();
    conversion->Convert();

    return 0;
}