#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>

using namespace std;

char* arrChars;
int arrSize;
ofstream file;

struct ArgsCollection
{
    ArgsCollection() : repeat(-1), maxRepeat(-1), repetitionCount(0), lastRepeatIndex(NULL), files(1), filesCount(1), charsMin(3), charsMax(5), fileSize(0), currentSize(0), lines(0), lineCount(0)
    { 
        fileName = const_cast<char*>("alist.txt");
    }
    
    char * fileName;
    int repeat, maxRepeat, repetitionCount;
    char * lastRepeatIndex;
    int files, filesCount, charsMin, charsMax;
    unsigned long long int fileSize, currentSize, lines, lineCount;
    
    bool checkRepetitionRepeat()
    {
        return (maxRepeat > -1 && repetitionCount >= maxRepeat);
    }
    
    void pushRepetitionCharIndex(char c)
    {
        char * temp = new char[repetitionCount];
        
        if (lastRepeatIndex != NULL)
            memcpy(temp, lastRepeatIndex, repetitionCount-1); 
        
        temp[repetitionCount-1] = c;
        
        if (lastRepeatIndex != NULL)
            delete[] lastRepeatIndex;
        
        lastRepeatIndex = temp;
    }
    
    void popRepetitionCharIndex(char c)
    {
        if (lastRepeatIndex != NULL && repetitionCount == 0)
        {
            delete[] lastRepeatIndex;
            lastRepeatIndex = NULL;
            return;
        }

        char * temp = new char[repetitionCount];
        
        bool skipped = false;
        for (int i = 0; i < (repetitionCount+1); ++i)
        {
            if (lastRepeatIndex[i] == c)
            {
                skipped = true;
                continue;
            }
            
            temp[skipped ? (i-1) : i] = lastRepeatIndex[i];
        }
        
        delete[] lastRepeatIndex;
        
        lastRepeatIndex = temp;
    }
    
    bool isInRepetition(char c)
    {
        if (lastRepeatIndex == NULL || repetitionCount < 1)
            return false;
        
        for (int i = 0; i < repetitionCount; ++i)
            if (arrChars[lastRepeatIndex[i]] == c)
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
        
        if (sArgs.lastRepeatIndex != NULL && sArgs.checkRepetitionRepeat() && sArgs.isInRepetition(str[strIndex]))
        {
            --sArgs.repetitionCount;
            sArgs.popRepetitionCharIndex(charsIndex);
        }
        
        for (int i = 0; i < size; ++i)
        {
            if (str[i] == arrChars[charsIndex])
                ++num;

            if (num > sArgs.repeat && sArgs.lastRepeatIndex == NULL && sArgs.repetitionCount < sArgs.maxRepeat)
            {
                ++sArgs.repetitionCount;
                sArgs.pushRepetitionCharIndex(charsIndex);
            }
            
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
                (sArgs.fileSize < 1 && sArgs.lines > 0 && (sArgs.lineCount+1) > ((sArgs.lines / sArgs.files) * sArgs.filesCount))) // ie offset etc
            {
                ++sArgs.filesCount;
                if (file.is_open())
                    file.close();
            }
         
        if (!file.is_open())
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
            cout << "Finished successfully! Wrote: " << double(sArgs.currentSize/pow(1024,2)) << " MBytes" << endl;
            exit(0);
        }
        
        file.write(str, size);
        file.put('\n');
        sArgs.currentSize += (size + 1);
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

void generateStrings()
{
    char * str;
    for (int i = sArgs.charsMin; i <= sArgs.charsMax; ++i)
    {
        str = new char[i];
        memset(str, 0, i);
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
    bool useLowCaps = false, useUpperCaps = false, useNumbers = false, useRandCharacters = false;
    
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
                sArgs.maxRepeat = atoi(argv[i+1]);
                break;
            case 97: // 'a'
                sArgs.charsMin = atoi(argv[i+1]);
                break;
            case 98: // 'b'
                sArgs.charsMin = atoi(argv[i+1]);
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
        }
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
        << "-o example.txt | Output to a file name" << endl
        << "-s 5           | Split to equal number of files" << endl
        << "-r 2           | Repeat the same character only a number of times in a row" << endl
        << "-R 1           | Only used when -r is used, number of characters repetition per combination allowed (limits repetition itself) i.e. (if param is 1) b3a9k23, 001234, EE9TPL" << endl
        << "-a 4           | Minimal number of characters to begin with" << endl
        << "-b 9           | Maximum number of characters to end with" << endl
        << "-l 12G         | Maximum size of your file in Gigabytes" << endl
        << "-L 15000       | Total number of lines to be generated" << endl
        << "-c             | Use lowercaps letters" << endl
        << "-C             | Use uppercaps letters" << endl
        << "-1             | Use numbers" << endl
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
            cout << "Use -h to see a list of available options" << endl;
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
        cout << "Finished successfully! Wrote: " << double(sArgs.currentSize/pow(1024,2)) << " MBytes" << endl;
    }
    catch (exception& e)
    {
        cout << e.what() << endl;
    }
    return 0;
}