#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>

using namespace std;

char* arrChars;
int arrSize;
ofstream file;

struct ArgsCollection
{
    ArgsCollection() : repeat(-1), files(1), min_num(2), max_num(5), file_size(0), current_size(0)
    { 
        fileName = const_cast<char*>("alist.txt");
    }
    
    char * fileName;
    int repeat;
    unsigned int files, min_num, max_num;
    unsigned long long int file_size, current_size;
};

ArgsCollection sArgs;

void generateCharacters(char * str, int size, int index);

void generateNextCharacter(char * str, int size, int strIndex, char charsIndex)
{
    if (sArgs.repeat > -1)
    {
        int num = 0;
        for (int i = 0; i < arrSize; ++i)
        {
            if (str[i] == arrChars[charsIndex])
                ++num;
                
            if (num > sArgs.repeat)
                return;
        }
    }
    
    str[strIndex] = arrChars[charsIndex];
    
    if (str[0] != 0)
    {
        if (!file.is_open())
            file.open(sArgs.fileName, ios::app);
        
        if (sArgs.current_size > 0 && sArgs.current_size >= sArgs.file_size)
        {
            cout << "Finished successfully! Wrote: " << double(sArgs.current_size/pow(1024,2)) << " MBytes" << endl;
            exit(0);
        }
        
        file.write(str, size);
        file.put('\n');
        sArgs.current_size += (size + 1);
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
    for (int i = sArgs.min_num; i <= sArgs.max_num; ++i)
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

void generateDictionaryArray(int argc, char ** argv)
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
            case 97: // 'a'
                sArgs.min_num = atoi(argv[i+1]);
                break;
            case 98: // 'b'
                sArgs.max_num = atoi(argv[i+1]);
                break;
            case 108: // 'l'
                {
                    sArgs.file_size = atoi(argv[i+1]);
                    string unit = argv[i+1];
                        
                    switch (unit.back())
                    {
                        case 75: // 'K'
                        case 107:
                            sArgs.file_size *= 1024;
                            break;
                        case 77: // 'M'
                        case 109:
                            sArgs.file_size *= pow(1024, 2);
                            break;
                        case 71: // 'G'
                        case 103:
                            sArgs.file_size *= pow(1024, 3);
                            break;
                        default:
                            cout << "Wrong file size Unit given, use G for Gigabyte(s), M for Megabyte(s), K for Kilobyte(s). Example: 1G == 1 Gigabyte" << endl; 
                            exit(0);
                            break;
                    }
                }
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
}

void showHelp()
{
    cout << "Options (with example of input):" << endl 
        << "-o example.txt | Output to a file name" << endl
        << "-s 5           | Split to equal number of files" << endl
        << "-r 2           | Repeat the same character only a number of times in a row" << endl
        << "-a 4           | Minimal number of characters to begin with" << endl
        << "-b 9           | Maximum number of characters to end with" << endl
        << "-l 12G         | Maximum size of your file in Gigabytes" << endl
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
        
        generateDictionaryArray(argc, argv);
        generateStrings();
        cout << "Finished successfully! Wrote: " << double(sArgs.current_size/pow(1024,2)) << " MBytes" << endl;
    }
    catch (exception& e)
    {
        cout << e.what() << endl;
    }
    return 0;
}