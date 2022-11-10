// A command-line interface program created to filter banned words from sample text files. Created by Will Corkill
// The libraries are necessary for program functionality
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>

void strToLower(std::string& sample)
/*
The intention of the function is to convert a string to be completely lower case.

This must be done character-by-character as C++ does not facilitate string
conversions by default
*/
{
    for (int i = 0; i < sample.size(); i++) // Used to identify each character in the string
    {
        sample[i] = tolower(sample[i]); // Convert the character to lower case
    }
}

void strRemovePunct(std::string& sample)
/*
The function will be used to remove any punctuation.
*/
{
    for (int i = 0; i < sample.size(); i++)
    {
        if (std::ispunct(sample[i]) && sample[i] != '-')
        {
            sample.erase(i, 1); // 1 is needed to specify the number of characters to be removed from the index position
            i--; // The iterator needs to be decreased to compensate for the loss of string length
        }
    }
}
std::vector< std::string > split(const std::string& str, char separator)
/*
The function 'split' is used to split a string into separate items in a vector.
The code from this function is taken from Techie Delight (Published 2018, Accessed 17 April 2022)
https://www.techiedelight.com/split-a-string-into-a-vector-in-cpp/
*/
{
    std::vector<std::string> tokens;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(separator, end)) != std::string::npos)
    {
        end = str.find(separator, start);
        tokens.push_back(str.substr(start, end - start));
    }
    return tokens;
}

struct textPunctuation
    // The structure is used to store the position of punctuation in the text
{
    bool replace; // Will be used to dictate whether the position will be replaced or inserted after
    int wordIndex; // The index of the word in the text file
    int position; // The position of the punctuation in the word
    char character; // The character to be stored
};

void sortStringArray(std::string* stringArray, int arraySize)
// An insertion sort is used to sort the values in the array
{
    int j;
    for (int i = 1; i < arraySize; i++)
    {
        std::string insert = stringArray[i];
        j = i;
        while ((j > 0) && (stringArray[j - 1] > insert))
        {
            stringArray[j] = stringArray[j - 1];
            j = j - 1;
        }
        stringArray[j] = insert;
    }
}


void readWordsFromFile(std::vector< std::string >& wordVector, std::vector< textPunctuation >& punctuationVector, std::string filePath, bool excludeOutput = false) // Setting a value to the parameters will act as a 'default' value
// wordVector is passed by reference so less memory is consumed by the program.
// The function will read the banned words into a vector, so that the size of the object can change during run-time.
{
    std::ifstream wordFile(filePath);
    if (!wordFile)
    {
        std::cout << "The file at path " << filePath << " could not be opened.\n";
        exit(0); // Do not proceed if the program can not open the file.
    }
    else
    {
        std::string word; // A variable will be needed to store the banned word when getline() is used

        std::ofstream outputFile("Sorted/unsorted.txt", std::ios_base::app);

        while (getline(wordFile, word)) // Ensure all lines are read into the vector
        {
            std::vector< std::string> separatedWords = split(word, ' '); // The split function is used to separate lines with spaces into separate elements in the vector
            for (int i = 0; i < separatedWords.size(); i++) // Add each word to the vector containing all the words in the file
            {
                for (int j = 0; j < separatedWords[i].size(); j++)
                {
                    if (std::ispunct(separatedWords[i][j]) && separatedWords[i][j] != '-') // Check if the character is punctuation
                    {
                        textPunctuation punct{ false, i, j, separatedWords[i][j] }; // Store the punctuation for later use
                        punctuationVector.push_back(punct);
                    }
                    else if (isupper(separatedWords[i][j])) // Check if the character is an upper case letter
                    {
                        textPunctuation punct{ true, i, j, separatedWords[i][j] };
                        punctuationVector.push_back(punct); // Store the character for later use
                    }
                }
                strRemovePunct(separatedWords[i]); // Remove the punctuation for the word vector
                strToLower(separatedWords[i]); // Convert the string to lower case for the word vector
                if (!excludeOutput) // Check whether to add the content to unsorted.txt
                {
                    outputFile << separatedWords[i] << " ";
                }
                wordVector.push_back(separatedWords[i]); // Add the processed word to the vector
            }
        }

        outputFile.close(); // Close the unsorted.txt file
        wordFile.close(); // Close the file to ensure good practice
    }
}

std::string censorWord(std::string textSample, std::vector< std::string >& censorSample)
// All parameters are passed by reference as it is less intense on memory
{
    for (int i = 0; i < censorSample.size(); i++) // Go through each word
    {
        std::size_t foundWordIndex = textSample.find(censorSample[i]);
        if (foundWordIndex != std::string::npos)
        {
            for (int j = 1; j < censorSample[i].length() - 1; j++) // Non-traditional start value of 1 is used to ensure the content in the middle is only redacted (not the end characters)
            {
                textSample[foundWordIndex + j] = '*'; // Replace the character with an asterisk
            }
        }
    }
    return textSample; // Return the modified sample
}
void addPunctuation(int wordPos, std::string& textSample, std::vector< textPunctuation >& punctSample)
// The function will return punctuation and capital letters to the word provided
{
    for (int i = 0; i < punctSample.size(); i++) // Check all punctuation entries in the vector
    {
        if (punctSample[i].wordIndex == wordPos) // If the word index matches, otherwise, discard
        {
            if (punctSample[i].replace && textSample[punctSample[i].position] != '*') // Check the character hasn't already been replaced with an asterisk
            {
                textSample[punctSample[i].position] = punctSample[i].character;
            }
            else if (!punctSample[i].replace) // Insert the character after the position if 'replace' is set to false
            {
                std::string punctuatedString = std::string(1, punctSample[i].character);
                textSample.insert(punctSample[i].position, punctuatedString);
            }
        }
    }
}

class FileImport
    /*
    The class is used to manage files which are imported for the program.

    Multiple attributes and methods will be used to make sure the code is
    as efficient and reusable as possible.
    */
{
public:
    // Attributes
    // Instantiation Method (runs when a class is created)
    FileImport(std::string path, bool excludeOutput = false) // An optional parameter is used so an output file isn't generated if not necessary
    {
        readWordsFromFile(words, punctuation, path, excludeOutput); // Get the words from the file and add to the relvant vectors
    }
    // Normal Methods
    int GetWordCount() // Used as a conduit between the accessible and inaccessible attributes of the class
    {
        return words.size();
    }
    std::vector< std::string >& GetWordVector() // Returns a private copy of the word vector
    {
        return words;
    }
    void Output(bool listWords = false) // Will output the information in the console
    {
        const std::string outputSeparator = listWords == true ? "\n" : " "; // A ternary operator is used to select the separation between items.
        for (int index = 0; index < words.size(); index++)
        {
            std::cout << words[index] << outputSeparator;
        }
    }
    void FilterText(std::vector< std::string >& bannedWords, std::string outputFileName = "") // Filter the content of the file. The output file is optional
    {
        std::ofstream outputFile;
        if (outputFileName != "") // Output the content to a file, if one is specified
        {
            outputFile.open("Filtered/" + outputFileName);
        }
        for (int i = 0; i < words.size(); i++)
        {
            std::string censoredWord = censorWord(words[i], bannedWords); // Censor the word
            addPunctuation(i, censoredWord, punctuation); // Add the punctuation back to the word after censoring
            std::cout << censoredWord << " ";
            if (outputFileName != "")
            {
                outputFile << censoredWord << " ";
            }
        }
    }
    void GetTopWords(int topWordCount, std::string* topWordArray) // Gets the specified number of top words in the file
    {
        std::map< int, std::vector< std::string > > counterMap = GetIndivWordCount(); // Gets the quantity of each word individually
        int iteration = 0;
        for (auto vector = counterMap.rbegin(); vector != counterMap.rend(); vector++) {
            for (auto word : vector->second)
            {
                if (iteration == topWordCount) // Break the loop if the number of top words has been reached
                {
                    break;
                }
                topWordArray[iteration] = word; // Set the top word in the array
                iteration++;
            }
        }
    }
private:
    // Attributes (private, as they should not be editable by the outside code)
    std::string filePath;
    std::vector< std::string > words;
    std::vector< textPunctuation > punctuation;
    // Methods
    std::map< int, std::vector< std::string > > GetIndivWordCount() // Returns the number of time each word occurs in the file
    {
        std::map< std::string, int> wordCounts; // First each word is counted
        for (int i = 0; i < words.size(); i++)
        {
            wordCounts[words[i]]++;
        }

        std::map< int, std::vector< std::string > > valueOrderedCounter; // Each is then added to a vector with the key of the number of times the word occurs
        for (auto i = wordCounts.begin(); i != wordCounts.end(); i++)
        {
            valueOrderedCounter[i->second].push_back(i->first);
        }

        return valueOrderedCounter;
    }
};

int main()
{
    std::cout << "+-----+\nBanned Words Filter\n+-----+\n";
    std::ofstream outputFile("Sorted/unsorted.txt", std::ios_base::out | std::ios_base::trunc);
    outputFile.close();

    /*
    PASS CRITERIA
    */
    std::cout << "\n\n+---------------+\n| Pass criteria |\n+---------------+\n";

    // Read in the banned words
    std::cout << "\nReading banned words into the program from the file:\n";

    FileImport bannedWordFile("Files/banned.txt", true);
    std::vector< std::string > bannedWordVector = bannedWordFile.GetWordVector();

    bannedWordFile.Output(true);

    // Read in the contents of text1.txt
    std::cout << "\nReading the words from text1.txt into the program:\n";

    FileImport text1File("Files/text1.txt");
    const std::vector< std::string > text1Vector = text1File.GetWordVector();

    text1File.Output();
    std::cout << "\n\nCounting the number of occurances of each word:\n";

    int* text1ComparisonResults = new int[bannedWordFile.GetWordCount()]; // A pointer is used as bannedWordFile is created during run-time
    for (int i = 0; i < bannedWordFile.GetWordCount(); i++)
    {
        text1ComparisonResults[i] = 0;
        for (int j = 0; j < text1File.GetWordCount(); j++)
        {
            if (bannedWordVector[i] == text1Vector[j])
            {
                text1ComparisonResults[i]++;
            }
        }
        std::cout << " - " << bannedWordVector[i] << " appears " << text1ComparisonResults[i] << " time(s).\n";
    }

    /*
    2:2 CRITERIA
    */
    std::cout << "\n\n+--------------+\n| 2:2 criteria |\n+--------------+\n";

    // Filter the text from text1.txt
    std::cout << "\nFiltering the text from text1.txt:\n\n";
    text1File.FilterText(bannedWordVector, "text1Filtered.txt");

    /*
    2:1 CRITERIA
    */
    std::cout << "\n\n+--------------+\n| 2:1 criteria |\n+--------------+\n";
    std::cout << "The filtration can now detect words within other words, regardless of case sensitivity.\n";

    // Filter the text from text2.txt
    std::cout << "\nFiltering the text from text2.txt:\n\n";

    FileImport text2File("Files/text2.txt");
    text2File.FilterText(bannedWordVector, "text2Filtered.txt");

    // Filter the text from text3.txt
    std::cout << "\nFiltering the text from text3.txt:\n\n";

    FileImport text3File("Files/text3.txt");
    text3File.FilterText(bannedWordVector, "text3Filtered.txt");

    // Filter the text from text4.txt
    std::cout << "\nFiltering the text from text4.txt:\n\n";

    FileImport text4File("Files/text4.txt");
    text4File.FilterText(bannedWordVector, "text4Filtered.txt");

    /*
    1ST CRITERIA
    */
    std::cout << "\n\n+--------------+\n| 1st criteria |\n+--------------+\n";
    std::cout << "The filtration has been updated to filter text more precisely.\n\nNow displaying the top 10 words from each file.\n";

    // Get the top words as necessary
    const int defaultTopWordSize = 10;

    // text1.txt
    std::string text1TopWords[defaultTopWordSize];
    text1File.GetTopWords(defaultTopWordSize, text1TopWords);
    sortStringArray(text1TopWords, defaultTopWordSize);

    std::cout << "\ntext1.txt:\n";
    for (int i = 0; i < defaultTopWordSize; i++)
    {
        std::cout << i + 1 << ") " << text1TopWords[i] << "\n";
    }
    // text2.txt
    std::string text2TopWords[defaultTopWordSize];
    text2File.GetTopWords(defaultTopWordSize, text2TopWords);
    sortStringArray(text2TopWords, defaultTopWordSize);

    std::cout << "\ntext2.txt:\n";
    for (int i = 0; i < defaultTopWordSize; i++)
    {
        std::cout << i + 1 << ") " << text2TopWords[i] << "\n";
    }
    // text3.txt
    std::string text3TopWords[defaultTopWordSize];
    text3File.GetTopWords(defaultTopWordSize, text3TopWords);
    sortStringArray(text3TopWords, defaultTopWordSize);

    std::cout << "\ntext3.txt:\n";
    for (int i = 0; i < defaultTopWordSize; i++)
    {
        std::cout << i + 1 << ") " << text3TopWords[i] << "\n";
    }
    // text4.txt
    std::string text4TopWords[defaultTopWordSize];
    text4File.GetTopWords(defaultTopWordSize, text4TopWords);
    sortStringArray(text4TopWords, defaultTopWordSize);

    std::cout << "\ntext4.txt:\n";
    for (int i = 0; i < defaultTopWordSize; i++)
    {
        std::cout << i + 1 << ") " << text4TopWords[i] << "\n";
    }

    // Outputting top words from all 10 files
    FileImport combinedFile("Sorted/unsorted.txt", true);

    std::string combinedTopWords[defaultTopWordSize];
    combinedFile.GetTopWords(defaultTopWordSize, combinedTopWords);
    sortStringArray(combinedTopWords, defaultTopWordSize);

    std::cout << "\nAll files combined:\n";
    for (int i = 0; i < defaultTopWordSize; i++)
    {
        std::cout << i + 1 << ") " << combinedTopWords[i] << "\n";
    }

    /*
    HIGH 1st CRITERIA
    */
    std::cout << "\n+-------------------+\n| High 1st criteria |\n+-------------------+\n";
    std::vector< std::string > combinedWords = combinedFile.GetWordVector();

    std::string* wordArray = new std::string[combinedWords.size()];
    for (int i = 0; i < combinedWords.size(); i++)
    {
        wordArray[i] = combinedWords[i];
    }

    sortStringArray(wordArray, combinedWords.size());

    std::ofstream sortedFile("Sorted/sorted.txt", std::ios_base::out | std::ios_base::trunc); // Will clear the file every time the program is run
    for (int i = 0; i < combinedWords.size(); i++)
    {
        sortedFile << wordArray[i] << " ";
    }
    sortedFile.close();

    return 0;
}
