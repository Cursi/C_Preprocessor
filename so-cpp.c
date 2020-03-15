// Amazing includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Amazing defines
#define MAX_LINE_LENGTH 256
#define INITIAL_CAPACITY 2
#define REALLOC_CAPACITY_RATE 2
#define INCLUDE_DIR_STARTING_KEY 'a'
#define ERROR_EXIT_CODE -1
#define ENOMEM_EXIT_CODE 12

// Dictionary strcture
typedef struct DictionaryEntry {
	char key[MAX_LINE_LENGTH];
	char value[MAX_LINE_LENGTH];
	struct DictionaryEntry *next;
} DictionaryEntry;

// Input code structure
typedef struct Code {
	char **lines;
	int usedCapacity;
	int maximumCapacity;
} Code;

void WriteOutputFile(DictionaryEntry **dictionary,
		     DictionaryEntry **includeDirs, Code *inputCode,
		     char *inputFilePath, char *outputFilePath);
Code *GetInputCode(char *inputFilePath);

// Returns the include input code entity
Code *TryDirs(DictionaryEntry *head, char *includeFileName)
{
	Code *includeInputCode;

	// Iterates through the includes from arguments
	while (head != NULL) {
		strcat(head->value, "/");
		strcat(head->value, includeFileName);

		// And tries to read foreach dir path
		includeInputCode = GetInputCode(head->value);
		if (includeInputCode != NULL)
			return includeInputCode;

		head = head->next;
	}

	// NULL for base case, if no path is correct
	return NULL;
}

// Frees the memory allocated for the input code entity
void FreeInputCode(Code *inputCode)
{
	int i;

	for (i = 0; i < inputCode->maximumCapacity; i++) {
		free(inputCode->lines[i]);
	}

	free(inputCode->lines);
	free(inputCode);
}

// Frees the memory allocated for the dictionary
void FreeDictionary(DictionaryEntry **head)
{
	DictionaryEntry *auxNode = *head, *nodeToBeFreed;

	while (auxNode != NULL) {
		nodeToBeFreed = auxNode;
		auxNode = auxNode->next;
		free(nodeToBeFreed);
	}
}

// Replaces the first searchBy occurance in str with replaceWith
int ReplaceSubstring(char **str, char *searchBy, char *replaceWith)
{
	char *p, *auxStr = *str;
	char buffer[MAX_LINE_LENGTH];

	// Return 0 if searchBy is not found for recursion
	p = strstr(auxStr, searchBy);
	if (p == NULL)
		return 0;

	// Copy starting characters
	strncpy(buffer, auxStr, p - auxStr);
	// Set the new ending string character
	auxStr[p - auxStr] = '\0';

	// Copy the replaceWith string and the rest of the original string
	sprintf(buffer + (p - auxStr), "%s%s", replaceWith,
		p + strlen(searchBy));
	// Copy back intro the original buffer
	strcpy(auxStr, buffer);

	// Return 1 on successful replace
	return 1;
}

// Returns the value mapped to a specific key
char *GetValueFromDictionary(DictionaryEntry *head, char *key)
{
	ReplaceSubstring(&key, "\t", "");
	ReplaceSubstring(&key, "\r", "");
	ReplaceSubstring(&key, "\n", "");

	// Iterate through the dictionary
	while (head != NULL) {
		// If there's a match with the key then return the value
		if (!strcmp(head->key, key))
			return head->value;

		head = head->next;
	}

	// Base case for key not found
	return NULL;
}

// Removes the key-value pair from dictionary
void RemoveDictionaryValue(DictionaryEntry **head, char *key)
{
	DictionaryEntry *auxNode = *head, *nodeBefore;

	// Base case for the first element
	if (auxNode != NULL && !strcmp(auxNode->key, key)) {
		*head = auxNode->next;
		free(auxNode);
	} else {
		// Iterates through the dictionary
		while (auxNode != NULL) {
			// If there's a match with the key then remove pair
			if (!strcmp(auxNode->key, key)) {
				nodeBefore->next = auxNode->next;
				free(auxNode);
				break;
			}

			// Save the node before and continue
			nodeBefore = auxNode;
			auxNode = auxNode->next;
		}
	}
}

// Adds the key-value pair in dictionary
void AddDictionaryValue(DictionaryEntry **head, char *key, char *value)
{
	DictionaryEntry *auxNode = *head, *newNode;
	int keyFound = 0;

	// Create new DictionaryEntry
	newNode = (DictionaryEntry *)malloc(sizeof(DictionaryEntry));
	if (newNode == NULL)
		exit(ENOMEM_EXIT_CODE);
	newNode->next = NULL;
	strcpy(newNode->key, key);
	strcpy(newNode->value, value);

	// Set it as first element if base case
	if (*head == NULL)
		*head = newNode;
	else {
		// Iterate through values
		while (auxNode->next != NULL) {
			// If there's a match with the key then refresh value
			if (!strcmp(auxNode->key, key)) {
				strcpy(auxNode->value, value);
				free(newNode);
				keyFound = 1;
			}

			auxNode = auxNode->next;
		}

		// If there's no match with the key then add the new pair
		if (!keyFound)
			auxNode->next = newNode;
	}
}

// Returns the a new code entity
Code *GetNewCode()
{
	int i;
	Code *newCode;

	newCode = (Code *)malloc(1 * sizeof(Code));

	if (newCode == NULL)
		exit(ENOMEM_EXIT_CODE);
	newCode->usedCapacity = 0;
	newCode->maximumCapacity = INITIAL_CAPACITY;

	// Alloc memory for the wrapper and foreach line
	newCode->lines = (char **)malloc(INITIAL_CAPACITY * sizeof(char *));
	if (newCode->lines == NULL)
		exit(ENOMEM_EXIT_CODE);
	for (i = 0; i < INITIAL_CAPACITY; i++) {
		newCode->lines[i] =
			(char *)malloc(MAX_LINE_LENGTH * sizeof(char));
		if (newCode->lines[i] == NULL)
			exit(ENOMEM_EXIT_CODE);
	}

	return newCode;
}

// Returns the realloced input code entity holding the parsed input
Code *GetReallocedInputCode(Code *inputCode)
{
	int i;
	// Multiply the presumed capacity
	inputCode->maximumCapacity *= REALLOC_CAPACITY_RATE;
	// Realloc the memory for the input code entity
	inputCode->lines = (char **)realloc(
		inputCode->lines, inputCode->maximumCapacity * sizeof(char *));
	if (inputCode->lines == NULL)
		exit(ENOMEM_EXIT_CODE);

	// Alloc memory foreach newly added line
	for (i = inputCode->usedCapacity; i < inputCode->maximumCapacity; i++) {
		inputCode->lines[i] =
			(char *)malloc(MAX_LINE_LENGTH * sizeof(char));
		if (inputCode->lines[i] == NULL)
			exit(ENOMEM_EXIT_CODE);
	}

	return inputCode;
}

// Returns the input code entity holding the parsed input
Code *GetInputCode(char *inputFilePath)
{
	FILE *inputFile;
	Code *inputCode = NULL;
	char currentLine[MAX_LINE_LENGTH];

	// Picks between a specified input path and stdin
	inputFile = strlen(inputFilePath) ? fopen(inputFilePath, "r") : stdin;

	// Checks for fopen call errors
	if (inputFile != NULL) {
		inputCode = GetNewCode();

		// Copies the lines into the input code entity
		while (fgets(currentLine, MAX_LINE_LENGTH, inputFile) != NULL) {
			strcpy(inputCode->lines[inputCode->usedCapacity],
			       currentLine);
			(inputCode->usedCapacity)++;

			if (inputCode->usedCapacity ==
			    inputCode->maximumCapacity)
				inputCode = GetReallocedInputCode(inputCode);
		}

		// Closes the file when finished
		fclose(inputFile);
	}

	return inputCode;
}

// Determines if a string is an empty line or not
int IsEmptyLine(char *currentLine)
{
	int i;

	for (i = 0; i < strlen(currentLine); i++) {
		if (currentLine[i] != ' ' && currentLine[i] != '\t' &&
		    currentLine[i] != '\n' && currentLine[i] != '\r' &&
		    currentLine[i] != '\0')
			return 0;
	}

	return 1;
}

// There we go...Huh, here's where the party starts:
// Treats all define cases
void TreatDefine(DictionaryEntry **dictionary, Code *inputCode,
		 int currentLineIndex)
{
	char currentLineCopy[MAX_LINE_LENGTH];
	char currentKey[MAX_LINE_LENGTH];
	char currentValue[MAX_LINE_LENGTH] = "";
	char *currentToken;

	// If there's a #define line
	if (!strncmp(inputCode->lines[currentLineIndex], "#define", 7)) {
		// Create a copy of the line
		strcpy(currentLineCopy, inputCode->lines[currentLineIndex]);

		// Get the key from the #define
		currentToken = strtok(currentLineCopy, " ");
		currentToken = strtok(NULL, " ");
		strcpy(currentKey, currentToken);

		currentToken = strtok(NULL, " ");

		// If there's a multi line define scrap it properly
		// Man that was so bullshit to do...C is so dumb :(
		if (inputCode->lines[currentLineIndex]
				[strlen(inputCode->lines[currentLineIndex]) -
				     2] == '\\') {
			while (strcmp(currentToken, "\\\n")) {
				strcat(currentValue, currentToken);
				strcat(currentValue, " ");
				currentToken = strtok(NULL, " ");
			}

			while (inputCode->lines
				[currentLineIndex]
				[strlen(inputCode->lines[currentLineIndex]) -
					2] == '\\') {
				strcpy(currentLineCopy,
				       inputCode->lines[++currentLineIndex]);
				strtok(currentLineCopy, "\r\n");
				currentToken = strtok(currentLineCopy, " \\");

				while (currentToken != NULL) {
					strcat(currentValue, currentToken);
					strcat(currentValue, " ");
					currentToken = strtok(NULL, " \\");
				}
			}

			currentValue[strlen(currentValue) - 1] = '\0';
		}
		// If there's a single line define scrap it properly
		else {
			while (currentToken != NULL) {
				strcat(currentValue, currentToken);
				strcat(currentValue, " ");
				currentToken = strtok(NULL, " ");
			}

			strtok(currentValue, "\r\n");
		}

		// Add the key-value pair to the dictionary
		AddDictionaryValue(dictionary, currentKey, currentValue);
	}
	// If there's an undef line scrap it properly
	else if (!strncmp(inputCode->lines[currentLineIndex], "#undef", 6)) {
		currentToken = strtok(inputCode->lines[currentLineIndex], " ");
		currentToken = strtok(NULL, " ");

		strtok(currentToken, "\r\n");
		// Remove the key-pair value from the dictionary
		RemoveDictionaryValue(dictionary, currentToken);
	}
}

// So much fun...
// Treats all includes recursively
void TreatInclude(DictionaryEntry **dictionary, DictionaryEntry **includeDirs,
		  char *currentLine, char *inputFilePath, char *outputFilePath)
{
	Code *includeInputCode;
	char *includeFilePath;
	char includeFileNameBackup[MAX_LINE_LENGTH];
	int i;

	// If there's an #include line
	if (!strncmp(currentLine, "#include", 8)) {
		// Extract include file name
		strtok(currentLine, "\"");
		includeFilePath = strtok(NULL, "\"\r\n");

		// Create a backup
		strcpy(includeFileNameBackup, includeFilePath);

		// Replace the iput file name with include file name
		for (i = strlen(inputFilePath) - 1; i > 0; i--) {
			if (inputFilePath[i] != '/' && inputFilePath[i] != '\\')
				inputFilePath[i] = '\0';
			else
				break;
		}

		// Copy it back modified
		strcat(inputFilePath, includeFilePath);
		strcpy(includeFilePath, inputFilePath);

		// Try to get code from include line
		includeInputCode = GetInputCode(includeFilePath);

		// If it fails, try all of the given dirs as params
		if (includeInputCode == NULL)
			includeInputCode =
				TryDirs(*includeDirs, includeFileNameBackup);

		// If none could be open the exit with error code
		if (includeInputCode == NULL)
			exit(ERROR_EXIT_CODE);

		// Call write that will recursively call TreatInclude and so on
		WriteOutputFile(dictionary, includeDirs, includeInputCode,
				includeFilePath, outputFilePath);
		// Free the input code entity
		FreeInputCode(includeInputCode);
	}
}

// Return true or false based on if type and existing expression
int GetIfEvaluationValue(DictionaryEntry **dictionary, Code *inputCode,
			 int currentLineIndex)
{
	char *currentToken;
	int evaluationValue = 0;

	// If there's a #if or #elif line
	if ((!strncmp(inputCode->lines[currentLineIndex], "#if", 3) &&
	     inputCode->lines[currentLineIndex][3] == ' ') ||
	    !strncmp(inputCode->lines[currentLineIndex], "#elif", 5))
		//  Get the evaluation value as an integer
		evaluationValue = atoi(
			strchr(inputCode->lines[currentLineIndex], ' ') + 1);
	// If there's a #ifdef or #ifndef line
	else {
		currentToken =
			strtok(inputCode->lines[currentLineIndex], " \t();");
		currentToken = strtok(NULL, " \t();");

		// Try to get the value based on the wanted key
		// If NULL is returned then set it as:
		// 0 for #ifdef and 1 for #ifndef
		evaluationValue = GetValueFromDictionary(*dictionary,
							 currentToken) != NULL;
		if (!strncmp(inputCode->lines[currentLineIndex], "#ifndef", 6))
			evaluationValue = !evaluationValue;
	}

	return evaluationValue;
}

// Oh, boy...Another fun one:
// Treats all if cases
int TreatIf(DictionaryEntry **dictionary, Code *inputCode, int currentLineIndex,
	    int *lastEval)
{
	int originalCurrentLineIndex = currentLineIndex;

	// If there's a line starting with:
	// #if or #elif or #ifdef or #ifndef
	if (!strncmp(inputCode->lines[currentLineIndex], "#if", 3) ||
	    !strncmp(inputCode->lines[currentLineIndex], "#elif", 5) ||
	    !strncmp(inputCode->lines[currentLineIndex], "#ifdef", 5) ||
	    !strncmp(inputCode->lines[currentLineIndex], "#ifndef", 6)) {
		// printf("%s\n", inputCode->lines[currentLineIndex]);

		// Get the evaluation value for it and skip it
		*lastEval = GetIfEvaluationValue(dictionary, inputCode,
						 currentLineIndex);
		currentLineIndex++;

		// Then until #else or #elif or #endif is found count lines
		for (currentLineIndex = currentLineIndex;
		     currentLineIndex < inputCode->usedCapacity;
		     currentLineIndex++) {
			if (!strncmp(inputCode->lines[currentLineIndex],
				     "#else", 5) ||
			    !strncmp(inputCode->lines[currentLineIndex],
				     "#elif", 5) ||
			    !strncmp(inputCode->lines[currentLineIndex],
				     "#endif", 6))
				break;
		}

		// Return number of lines to be skipped if value was true
		// And 0 if the value was false
		return !(*lastEval) ?
			       currentLineIndex - originalCurrentLineIndex - 1 :
			       0;
	}

	// Else if there's a line starting with #else or #elif
	else if (!strncmp(inputCode->lines[currentLineIndex], "#else", 5) ||
		 !strncmp(inputCode->lines[currentLineIndex], "#elif", 5)) {
		// If the last evaluation value was true
		if (*lastEval) {
			// Count lines until #endif
			for (currentLineIndex = currentLineIndex;
			     currentLineIndex < inputCode->usedCapacity;
			     currentLineIndex++) {
				if (!strncmp(inputCode->lines[currentLineIndex],
					     "#endif", 6))
					break;
			}

			// Return the number of lines to be skipped
			return currentLineIndex - originalCurrentLineIndex;
		}
	}

	// Base case, return 0 if it's not a special line
	return 0;
}

// Writes the output scraped and modified
void WriteOutputFile(DictionaryEntry **dictionary,
		     DictionaryEntry **includeDirs, Code *inputCode,
		     char *inputFilePath, char *outputFilePath)
{
	FILE *outputFile;
	char currentLine[MAX_LINE_LENGTH];
	int i, oldI, lastIfEval = 0;
	char *pch, *currentSubstring, *existingValue;

	// Picks between a specified output path and stdout
	outputFile =
		strlen(outputFilePath) ? fopen(outputFilePath, "w") : stdout;

	// For each existing line
	for (i = 0; i < inputCode->usedCapacity; i++) {
		// If there is not a line starting with:
		// #undef or #ifdef or #ifndef
		if (strncmp(inputCode->lines[i], "#undef", 6) &&
		    strncmp(inputCode->lines[i], "#ifdef", 5) &&
		    strncmp(inputCode->lines[i], "#ifndef", 6)) {
			// Create a copy of the line
			strcpy(currentLine, inputCode->lines[i]);

			pch = strtok(currentLine, " \t();");
			currentSubstring = strstr(inputCode->lines[i], pch);

			// Apply for each word the key -> value transformation
			// if there's an existing entry in the dictionary
			while (pch != NULL) {
				currentSubstring =
					strstr(currentSubstring, pch);

				existingValue = GetValueFromDictionary(
					*dictionary, pch);
				if (existingValue != NULL)
					ReplaceSubstring(&currentSubstring, pch,
							 existingValue);

				pch = strtok(NULL, " \t();");
			}
		}

		// Call for treat includes
		TreatInclude(dictionary, includeDirs, inputCode->lines[i],
			     inputFilePath, outputFilePath);

		// Call for treat defines
		TreatDefine(dictionary, inputCode, i);

		// Save the old current line index
		oldI = i;
		// Increment the current line index with
		// the number of lines to be skipped returned by TreatIf
		i += TreatIf(dictionary, inputCode, i, &lastIfEval);

		// If there's a change about the current line index
		// then print the line if it's not a special one
		// Example: empty lines, define lines etc. are hidden
		if (oldI == i) {
			if (i + 2 < inputCode->usedCapacity &&
			    strstr(inputCode->lines[i], "\\\n") != NULL &&
			    strstr(inputCode->lines[i + 1], "\\\n") == NULL)
				i += 2;

			if (!IsEmptyLine(inputCode->lines[i]) &&
			    strncmp(inputCode->lines[i], "#define", 7) &&
			    strncmp(inputCode->lines[i], "#undef", 6) &&
			    strncmp(inputCode->lines[i], "#endif", 6) &&
			    strncmp(inputCode->lines[i], "#if", 3) &&
			    strncmp(inputCode->lines[i], "#elif", 5) &&
			    strncmp(inputCode->lines[i], "#else", 5) &&
			    strncmp(inputCode->lines[i], "#include", 8) &&
			    strstr(inputCode->lines[i], "\\\n") == NULL)
				fprintf(outputFile, "%s", inputCode->lines[i]);
		}
	}

	// If it's not the stdout where we write then close it
	if (outputFile != stdout)
		fclose(outputFile);
}

// Finally...The main function :O
int main(int argc, char *argv[])
{
	DictionaryEntry *dictionary = NULL;
	DictionaryEntry *includeDirs = NULL;
	Code *inputCode;
	char inputFilePath[MAX_LINE_LENGTH] = "";
	char outputFilePath[MAX_LINE_LENGTH] = "";
	int numberOfFileParams = 0, i;
	char *currentKey, *currentValue;
	char includeDirKey[1];

	includeDirKey[0] = INCLUDE_DIR_STARTING_KEY;

	// Iterate through arguments
	// Parse and store the wanted info
	for (i = 1; i < argc; i++) {
		if (strchr(argv[i], '-')) {
			if (argv[i][1] == 'D') {
				if (strlen(argv[i]) == 2)
					i++;
				else
					argv[i] += 2;
				if (strchr(argv[i], '=') != NULL) {
					currentKey = strtok(argv[i], "=");
					currentValue = strtok(NULL, "=");
					AddDictionaryValue(&dictionary,
							   currentKey,
							   currentValue);
				} else
					AddDictionaryValue(&dictionary, argv[i],
							   "");
			} else if (argv[i][1] == 'I') {
				if (strlen(argv[i]) == 2)
					i++;
				else
					argv[i] += 2;

				AddDictionaryValue(&includeDirs, includeDirKey,
						   argv[i]);
				includeDirKey[0]++;
			} else
				return ERROR_EXIT_CODE;
		} else {
			numberOfFileParams++;

			// Set the input file path
			if (numberOfFileParams == 1)
				strcpy(inputFilePath, argv[i]);
			// Set the output file path
			else if (numberOfFileParams == 2)
				strcpy(outputFilePath, argv[i]);
			else
				return ERROR_EXIT_CODE;
		}
	}

	// Get the input code entity or exit if error
	inputCode = GetInputCode(inputFilePath);
	if (inputCode == NULL)
		exit(ERROR_EXIT_CODE);

	// Write the output
	WriteOutputFile(&dictionary, &includeDirs, inputCode, inputFilePath,
			outputFilePath);

	// Free stuff
	FreeInputCode(inputCode);
	FreeDictionary(&dictionary);
	FreeDictionary(&includeDirs);

	// Return from the nasty road like a hero
	return 0;
}
