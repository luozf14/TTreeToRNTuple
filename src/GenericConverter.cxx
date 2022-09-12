#include "TTreeToRNTuple.hxx"

#include <string>
#include <vector>

static void Usage(char *progname)
{
    std::cout << "Usage: " << progname << " -i <input.root> -o <output.ntuple> -t(ree) <tree name> "
              << "[-d(ictionary) <dictionary names>] [-c(ompression) <compression algorithm>] [-m(t)]"
              << std::endl;
}

int main(int argc, char **argv)
{
    std::string inputFile;
    std::string outputFile;
    std::string treeName;
    std::string compressionAlgo = "none";
    std::vector<std::string> dictionaries = {};
    bool enableMtiltiThread = false;

    int inputArg;
    while ((inputArg = getopt(argc, argv, "hi:o:c:d:b:mt:")) != -1)
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
        case 'm':
            enableMtiltiThread = true;
            break;
        case 't':
            treeName = optarg;
            break;
        default:
            fprintf(stderr, "Unknown option: -%c\n", inputArg);
            Usage(argv[0]);
            return 1;
        }
    }

    if(inputFile.empty()||outputFile.empty()||treeName.empty())
    {
        printf("Error: Minimal required parameters: -i <input.root> -o <output.ntuple> -t(ree) <tree name>\n)");
        exit(0);
    }

    std::unique_ptr<TTreeToRNTuple> conversion = std::make_unique<TTreeToRNTuple>(inputFile, outputFile, treeName);
    conversion->SetCompressionAlgo(compressionAlgo);
    conversion->SetDictionary(dictionaries);
    conversion->EnableMultiThread(enableMtiltiThread);
    conversion->Convert();
    
    return 0;
}