#include "TTreeToRNTuple.hxx"

#include <string>
#include <vector>
#include <iostream>

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
        std::cerr<<"Error: Minimal required parameters: -i <input.root> -o <output.ntuple> -t(ree) <tree name>"<<std::endl;
        exit(1);
    }

    std::unique_ptr<TTreeToRNTuple> conversion = std::make_unique<TTreeToRNTuple>(inputFile, outputFile, treeName);
    conversion->SetCompressionAlgo(compressionAlgo);
    conversion->SetDictionary(dictionaries);
    conversion->SelectBranches(subBranches);
    if (flagDefaultProgressCallbackFunc)
        conversion->SetUserProgressCallbackFunc([](int current, int total)
                                                {
        int interval = total / 100 * 5;
        if (current % interval == 0)
        {
            fprintf(stderr, "\rProcessing entry %d of %d [\033[00;33m%2.1f%% completed\033[00m]",
                    current, total,
                    (static_cast<float>(current) / total) * 100);
        }
        if(current == total)
        {
            fprintf(stderr, "\rProcessing entry %d of %d [\033[00;32m%2.1f%% completed\033[00m]\n",
                    current, total,
                    (static_cast<float>(current) / total) * 100);
        } });
    conversion->Convert();

    return 0;
}