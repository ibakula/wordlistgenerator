#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>

using namespace std;

char* arrChars;
int arrSize;
fstream file;

struct ArgsCollection
{
    ArgsCollection() : repeat(-1), repetitionLimiter(-1), repetitionCount(0), repetitorArray(NULL), files(1), filesCount(1), charsMin(3), charsMax(5), fileSize(0), currentSize(0), lines(0), lineCount(0), output(false), hasFileCheckpoint(false)
    { 
        fileName = const_cast<char*>("alist.txt");
        start = NULL;
    }
    
    char * fileName, * start;
    int repeat, repetitionLimiter, repetitionCount;
    char * repetitorArray;
    int files, filesCount, charsMin, charsMax;
    unsigned long long int fileSize, currentSize, lines, lineCount;
    bool output, hasFileCheckpoint;
    
    bool checkRepetitionRepeat()
    {
        return (repetitionLimiter > -1 && repetitionCount >= repetitionLimiter);
    }
    
    void pushRepetitionCharIndex(char c)
    {
        ++repetitionCount;
        char * temp = new char[repetitionCount];
        
        if (repetitorArray != NULL)
        {
            memcpy(temp, repetitorArray, repetitionCount-1); 
            delete[] repetitorArray;
        }
        
        temp[repetitionCount-1] = c;
        
        repetitorArray = temp;
    }
    
    void popRepetitionCharIndex(char c)
    {
        --repetitionCount;
        
        if (repetitionCount == 0)
        {
            if (repetitorArray != NULL)
                delete[] repetitorArray;
            
            repetitorArray = NULL;
            return;
        }

        char * temp = new char[repetitionCount];
        
        bool skipped = false;
        for (int i = 0; i < (repetitionCount+1); ++i)
        {
            if (repetitorArray[i] == c)
            {
                skipped = true;
                continue;
            }
            
            temp[skipped ? (i-1) : i] = repetitorArray[i];
        }
        
        delete[] repetitorArray;
        
        repetitorArray = temp;
    }
    
    bool isInRepetition(char c)
    {
        if (repetitorArray == NULL || repetitionCount < 1)
            return false;
        
        for (int i = 0; i < repetitionCount; ++i)
            if (repetitorArray[i] == c)
                return true;
        
        return false;
    }
};

ArgsCollection sArgs;

void generateCharacters(char * str, int size, int index);

void generateNextCharacter(char * str, int size, int strIndex, char charsIndex)
{
    if (sArgs.repeat > -1)
    {
        int num = 0;
        
        if (sArgs.repetitorArray != NULL && sArgs.checkRepetitionRepeat() && sArgs.isInRepetition(charsIndex))
            sArgs.popRepetitionCharIndex(charsIndex);
        
        for (int i = 0; i < size; ++i)
        {
            if (str[i] == arrChars[charsIndex])
                ++num;

            if (num > sArgs.repeat && sArgs.repetitorArray == NULL && sArgs.repetitionCount < sArgs.repetitionLimiter)
                sArgs.pushRepetitionCharIndex(charsIndex);
            
            if (num > 0 && sArgs.checkRepetitionRepeat())
                return;
            
            if (num > sArgs.repeat)
                return;
        }
    }
    
    str[strIndex] = arrChars[charsIndex];
    
    if (str[0] != 0)
    {
        if (sArgs.files > 1 && sArgs.filesCount < sArgs.files)
            if ((sArgs.fileSize > 0 && (sArgs.currentSize + size + 1) > ((sArgs.fileSize / sArgs.files) * sArgs.filesCount)) ||
                (sArgs.lines > 0 && (sArgs.lineCount+1) > (sArgs.lines / sArgs.files))) // ie offset etc
            {
                if (sArgs.lines > 0 && (sArgs.lineCount+1) > ((sArgs.lines / sArgs.files) * sArgs.filesCount))
                    sArgs.lineCount = 0;
                
                ++sArgs.filesCount;
                if (file.is_open())
                    file.close();
            }
         
        if (!file.is_open() && (sArgs.fileName != "alist.txt" && !sArgs.output))
        {
            if (sArgs.files <= 1)
                file.open(sArgs.fileName, ios::app);
            else
            {
                string fileName = sArgs.fileName;
                size_t pos = fileName.find('.');
                
                if (pos != string::npos)
                    fileName.insert(pos, to_string(sArgs.filesCount));
                
                file.open(fileName, ios::app);
            }
        }
        
        if (sArgs.lines > 0)
            ++sArgs.lineCount;
        
        if ((sArgs.fileSize > 0 && sArgs.currentSize > 0 && sArgs.currentSize >= sArgs.fileSize) || (sArgs.lines > 0 && sArgs.lineCount > sArgs.lines))
        {
            cerr << "Finished successfully! Wrote: " << double(sArgs.currentSize/pow(1024,2)) << " MBytes" << endl;
            exit(0);
        }
        
        if (sArgs.fileName != "alist.txt" && !sArgs.output)
        {
            file.write(str, size);
            file.put('\n');
            sArgs.currentSize += (size + 1);
        }
        
        if (sArgs.output)
            cout << str << endl;
    }
    
    if ((strIndex+1) < size && (strIndex == 0 || str[strIndex-1] != 0))
        for (int i = (size-1); i > strIndex; --i)
        {
            generateCharacters(str, size, i);
            
            if (str+i != NULL && str[i] == arrChars[arrSize-1])
                str[i] = arrChars[0];
        }
}

int findCharIndex(char c)
{
    for (int i = 0; i < arrSize; ++i)
        if (c == arrChars[i])
            return i;
    
    return 0;
}

void generateCharacters(char * str, int size, int index)
{
    for (char i = (str[index] == arrChars[arrSize-1] || str[index] == 0 ? 0 : (findCharIndex(str[index])+1)); i < arrSize; ++i)
    {
        generateNextCharacter(str, size, index, i);
        
        if (str[index] == 0)
            continue;
        
        if (index > 0 && str[index-1] == 0)
            return;
    }
}

void generateString(char * str, int size)
{
    for (int i = (size-1); i > -1; --i)
        generateCharacters(str, size, i);
}

size_t getSizeOfStartStr()
{
    size_t itr = 0;
    
    while (sArgs.start[itr] != 0)
        ++itr;
    
    return itr;
}

void generateStrings()
{
    char * str;
    
    if (sArgs.start != NULL)
    {
        if (sArgs.hasFileCheckpoint)
        {
            file.open(sArgs.start, ios::in);
            string str2;
            
            if (file.is_open())
            {
                char c = 0;
                file.seekg(-2, file.end);

                do
                {
                    file.get(c);
                    if (file.tellg() < 2 || c == '\n')
                    {
                        if (file.tellg() == 1)
                            file.seekg(0, file.beg);
                        
                        std::getline(file, str2);
                        break;
                    }
                }
                while (file.seekg(int(file.tellg())-2));
            }
            
            file.close();
            
            if (!str2.empty())
            {
                sArgs.charsMin = str2.size();
                str = new char[sArgs.charsMin];
                memcpy(str, str2.c_str(), sArgs.charsMin);
            }
            else
            {
                sArgs.start = NULL;
                sArgs.hasFileCheckpoint = false;
                cerr << "Source file invalid, -t option ignored" << endl;
            }
        }
        else
        {
            size_t size = getSizeOfStartStr();
            
            if (size > 0)
            {
                str = new char[size];
                memcpy(str, sArgs.start, size);
                sArgs.charsMin = size;
            }
            else 
            {
                sArgs.start = NULL;
                cerr << "Invalid input, -T option ignored." << endl;
            }
        }
    }

    for (int i = sArgs.charsMin; i <= sArgs.charsMax; ++i)
    {
        if (sArgs.start == NULL)
        {
            str = new char[i];
            memset(str, 0, i);
        }
        else sArgs.start = NULL;
        
        generateString(str, i);
        delete[] str;
    }
}

void generateCharactersInASCIIRange(char first, char last)
{
    char* tempChars = NULL;
    if (arrChars != NULL)
    {
        tempChars = new char[arrSize];
        memcpy(tempChars, arrChars, arrSize);
        arrSize += int(last-first)+1;
        delete[] arrChars;
    }
    else
        arrSize = int(last-first)+1;
        
    arrChars = new char[arrSize];
        
    if (tempChars != NULL && arrSize > int(last-first))
        memcpy(arrChars, tempChars, (arrSize-int(last-first)+1));
        
    for (char i = first; i <= last; ++i)
        arrChars[arrSize-(int(last-i)+1)] = i;
        
    if (tempChars != NULL)
        delete[] tempChars;
}

bool generateDictionaryArray(int argc, char ** argv)
{
    bool useLowCaps = false, useUpperCaps = false, useNumbers = false, useRandCharacters = false, hasCheckpoint = false;
    
    for (int i = 0; i < argc; ++i)
    {
        switch (argv[i][1])
        {
            case 111: // 'o' 
                sArgs.fileName = argv[i+1];
                break;
            case 115: // 's'
                sArgs.files = atoi(argv[i+1]);
                break;
            case 114: // 'r'
                sArgs.repeat = atoi(argv[i+1]);
                break;
            case 82: // 'R'
                sArgs.repetitionLimiter = atoi(argv[i+1]);
                break;
            case 97: // 'a'
                sArgs.charsMin = atoi(argv[i+1]);
                break;
            case 98: // 'b'
                sArgs.charsMax = atoi(argv[i+1]);
                break;
            case 108: // 'l'
                {
                    sArgs.fileSize = atoi(argv[i+1]);
                    string unit = argv[i+1];
                        
                    switch (unit.back())
                    {
                        case 75: // 'K'
                        case 107:
                            sArgs.fileSize *= 1024;
                            break;
                        case 77: // 'M'
                        case 109:
                            sArgs.fileSize *= pow(1024, 2);
                            break;
                        case 71: // 'G'
                        case 103:
                            sArgs.fileSize *= pow(1024, 3);
                            break;
                        default:
                            cerr << "Wrong file size Unit given, use G for Gigabyte(s), M for Megabyte(s), K for Kilobyte(s). Example: 1G == 1 Gigabyte" << endl;
                            return false;
                            break;
                    }
                }
                break;
            case 76: // 'L'
                sArgs.lines = atoi(argv[i+1]);
                break;
            case 99: // 'c'
                useLowCaps = true;
                break;
            case 67: // 'C'
                useUpperCaps = true;
                break;
            case 49: // '1'
                useNumbers = true;
                break;
            case 36: // '$'
                useRandCharacters = true;
                break;
            case 118: // 'v'
                sArgs.output = true;
                break;
            case 116: // 't'
                sArgs.start = argv[i+1];
                sArgs.hasFileCheckpoint = true;
                break;
            case 84: // 'T'
                sArgs.start = argv[i+1];
                hasCheckpoint = true;
                break;
        }
    }
    
    if ((sArgs.hasFileCheckpoint || hasCheckpoint) && (sArgs.start == NULL || (sArgs.hasFileCheckpoint && sArgs.start[0] == '-')))
    {
        sArgs.hasFileCheckpoint = false;
        sArgs.start = NULL;
        cerr << "-t option ignored, please include a file output" << endl;
    }
    
    if (sArgs.files > 1 && sArgs.lines < 1 && sArgs.fileSize < 1)
    {
        sArgs.files = 1;
        cerr << "-s option ignored, please specify -l or -L to use this option" << endl;
    }
    
    if (!useLowCaps && !useUpperCaps && !useNumbers && !useRandCharacters)
        useLowCaps = true;
    
    if (useNumbers)
        generateCharactersInASCIIRange('0', '9');
    
    if (useUpperCaps)
        generateCharactersInASCIIRange('A', 'Z');
        
    if (useLowCaps)
        generateCharactersInASCIIRange('a', 'z');
        
    if (useRandCharacters)
    {
        generateCharactersInASCIIRange('!', '/');
        generateCharactersInASCIIRange(':', '@');
        generateCharactersInASCIIRange('[', '`');
        generateCharactersInASCIIRange('{', '~');
    }
    
    return true;
}

void showHelp()
{
    cout << "Options (with example of input):" << endl 
        << "-(Option) (Example Input) | (Comment)" << endl
        << "-1             | Use numbers" << endl
        << "-a 4           | Minimal number of characters to begin with" << endl
        << "-b 9           | Maximum number of characters to end with" << endl
        << "-c             | Use lowercaps letters" << endl
        << "-C             | Use uppercaps letters" << endl
        << "-l 12G         | Maximum size of your file in Gigabytes" << endl
        << "-L 15000       | Total number of lines to be generated" << endl
        << "-o example.txt | Output to a file name" << endl
        << "-r 2           | Repeat the same character only a number of times in a row" << endl
        << "-R 1           | Only used when -r is used, number of characters repetition per combination allowed (limits repetition itself) i.e. (if param is 1) b3a9k23, 001234, EE9TPL" << endl
        << "-s 5           | Split to equal number of files, -l arg has a higher priority than -L" << endl
        << "-t example.txt | Continue from last occurrence, -a option is ignored" << endl
        << "-T 11AE435     | Continue from input, -a option continues from inputs length" << endl
        << "-v             | Output generated, note that output to file is ignored unless -o is specifically defined" << endl
        << "-$             | Use $#&.. characters" << endl
        << "The example of arguments provided: " << endl
        << "cppnary -o example.txt -s 5 -r 2 -a 4 -b 9 -l 12G" << endl
        << "The program generates characters into 5 splitted files," << endl
        << "repeats the same character only 2 times in the whole string," << endl
        << "the program starts with 4 characters combination in its string array," << endl
        << "the program stops at 9 characters combination in its string array," << endl
        << "the filesize of the example.txt is 12 Gigabytes, when limit reached, the program stops at all costs" << endl
        << " DO NOT TRY COMBINING ANY OF THE ARGS, AS ITS NOT IMPLEMENTED " << endl;
}

int main(int argc, char ** argv)
{
    try
    {
        if (argc < 2)
        {
            cerr << "Use -h to see a list of available options" << endl;
            return 0;
        }
        
        if (argv[1][1] == 'h')
        {
            showHelp();
            return 0;
        }
        
        if (!generateDictionaryArray(argc, argv))
            return 0;
        
        generateStrings();
        cerr << "Finished successfully! Wrote: " << double(sArgs.currentSize/pow(1024,2)) << " MBytes" << endl;
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }
    return 0;
}