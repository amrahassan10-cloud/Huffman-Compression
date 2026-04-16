#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>


using namespace std;

// Node in Huffman Tree
struct Node {
    char ch;
    int freq;
    Node* left;
    Node* right;
};

// Priority Queue Node (linked list element)
struct PQNode {
    Node* treeNode;
    PQNode* next;
};

// Frequency Node (linked list for symbol frequencies)
struct FreqNode {
    int symbol;
    int freq;
    FreqNode* next;
};

void insertOrUpdate(FreqNode*& head, int symbol) {

    FreqNode* current = head;
    // here we are looking for a existing symbol in the list so we increment
    while (current != nullptr) {
        if (current->symbol == symbol) {
            current->freq++;
            return;
        }

        current = current->next;
    }

    // if symbol not found we create a new node for the symbol
    FreqNode* newNode = new FreqNode;
    newNode->symbol = symbol;
    newNode->freq = 1;
    newNode->next = head;
    head = newNode;

}

void insert(PQNode*& head, Node* newNode) {

    //creating a new priorty queue node 

    PQNode* newPQNode = new PQNode;
    newPQNode->treeNode = newNode;
    newPQNode->next = nullptr;

    // If queue empty or new node has smaller freq than head, insert at front

    if (head == nullptr || newNode->freq < head->treeNode->freq) {
        newPQNode->next = head;
        head = newPQNode;
        return;
    }

    //otherwise we need to find the insertion point because it might be in middle of two or at last
    PQNode* current = head;
    while (current->next && current->next->treeNode->freq <= newNode->freq) {
        current = current->next;
    }
    //insert it after current
    newPQNode->next = current->next;
    current->next = newPQNode;

}

void buildPriorityQueue(FreqNode* freqList, PQNode*& pq) {

    while (freqList != nullptr) {

        Node* newNode = new Node;
        newNode->ch = (char)freqList->symbol;
        newNode->freq = freqList->freq;
        newNode->left = nullptr;
        newNode->right = nullptr;

        insert(pq, newNode);
        freqList = freqList->next;

    }

}

// function to pop from the queue

Node* pop(PQNode*& head) {

    if (head == nullptr) {

        return nullptr;
    }

    PQNode* temp = head;
    Node* result = temp->treeNode;
    head = head->next;
    delete temp;
    return result;

}


Node* buildHuffmanTree(PQNode*& pq) {


    while (pq != nullptr && pq->next != nullptr)
    {
        //pop the smallest two

        Node* left = pop(pq);
        Node* right = pop(pq);

        //parent node with the freq combined

        Node* parent = new Node;
        parent->ch = '\0';
        parent->freq = left->freq + right->freq;
        parent->left = left;
        parent->right = right;

        //inset the node back into the queue
        insert(pq, parent);
    }

    //the remained node is the root for huffman
    return pop(pq);
}


void generateCodes(Node* root, char codes[256][256], char path[256], int depth) {


    if (root == nullptr) {
        return;
    }

    if (root->left == nullptr && root->right == nullptr) //that means that it is a leaf node
    {
        path[depth] = '\0';
        strcpy_s(codes[(unsigned char)root->ch], path);
        return;
    }
    
    //traverse left add 0

    path[depth] = '0';
    generateCodes(root->left, codes, path, depth + 1); // gemini helped us in the recurion part in generating the codes

    //traverse right add 1

    path[depth] = '1';
    generateCodes(root->right, codes, path, depth + 1);

}


// --- Save and Load Huffman Tree ---

// got the idea for save tree and load tree from geeksforgeeks

void saveTree(FILE* out, Node* root) {

    // If the node is null return
    if (root == nullptr) {
        return;
    }

    // If it's a leaf node 
    if (root->left == nullptr && root->right == nullptr)
    {
        // Write '1' to show this is a leaf node

        fputc('1', out);

        // Write the character stored in this leaf

        fputc(root->ch, out);
    }

    else
    {
        // Write '0' to show this is an internal node
        fputc('0', out);

        saveTree(out, root->left);
        saveTree(out, root->right);

    }

}

Node* loadTree(FILE* in) {

    int flag = fgetc(in);

    if (flag == EOF) return nullptr;

    if (flag == '1') {
        int ch = fgetc(in);
        if (ch == EOF) return nullptr;
        return new Node{ (char)ch, 0, nullptr, nullptr };
    }
    else if (flag == '0') {
        Node* left = loadTree(in);
        Node* right = loadTree(in);
        return new Node{ '\0', 0, left, right };
    }
    else {
        // Invalid format
        return nullptr;
    }
}


//  Read frequencies from file 

void readFrequencies(const char* filename, FreqNode*& freqList, size_t bufferSize) {

    FILE* file;
    if (fopen_s(&file, filename, "rb") != 0) {
        cout << "Error: Cannot open file.\n";
        exit(1);
    }
    unsigned char* buffer = new unsigned char[bufferSize];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, bufferSize, file)) > 0) {

        for (size_t i = 0; i < bytesRead; ++i) {
            insertOrUpdate(freqList, buffer[i]);
        }

    }

    delete[] buffer;
    fclose(file);

}

//  Compress file

void compressFile(const char* inputFile, const char* outputFile, Node* root, char codes[256][256], size_t bufferSize) {

    FILE* in;
    FILE* out;
    if (fopen_s(&in, inputFile, "rb") != 0 || fopen_s(&out, outputFile, "wb") != 0) {
        cout << " Failed to open files.\n";
        exit(1);
    }

    // Write Huffman tree at start of compressed file (bonus part)
    saveTree(out, root);

    unsigned char* inBuffer = new unsigned char[bufferSize];
    unsigned char* outBuffer = new unsigned char[bufferSize];
    size_t inRead, outIndex = 0;
    unsigned char bitBuffer = 0;
    int bitCount = 0;


    while ((inRead = fread(inBuffer, 1, bufferSize, in)) > 0) {

        for (size_t i = 0; i < inRead; ++i) {

            const char* code = codes[inBuffer[i]];

            for (int j = 0; code[j] != '\0'; ++j) {

                bitBuffer = (bitBuffer << 1) | (code[j] - '0'); // ai helped us in this part
                bitCount++;

                if (bitCount == 8) {

                    outBuffer[outIndex++] = bitBuffer;
                    bitBuffer = 0;
                    bitCount = 0;

                    if (outIndex == bufferSize) {

                        fwrite(outBuffer, 1, outIndex, out);
                        outIndex = 0;
                    }
                }
            }
        }
    }

    // padding
    if (bitCount > 0) {
        bitBuffer <<= (8 - bitCount);
        outBuffer[outIndex++] = bitBuffer;
    }

    if (outIndex > 0)
        fwrite(outBuffer, 1, outIndex, out);

    delete[] inBuffer;
    delete[] outBuffer;
    fclose(in);
    fclose(out);

    printf("Compression complete. Output written to %s\n", outputFile);

}

//  Cleanup  

void freeTree(Node* root) {

    if (root == nullptr) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;

}


//  Decompress file 

void decompressFile(const char* inputFile, const char* outputFile, size_t bufferSize) {

    FILE* in;
    FILE* out;
    if (fopen_s(&in, inputFile, "rb") != 0 || fopen_s(&out, outputFile, "wb") != 0) {
        cout << " Failed to open files.\n";
        exit(1);
    }

    // Load Huffman tree from start of compressed file
    Node* root = loadTree(in);
    if (!root) {
        cout << "Error: Failed to load Huffman tree.\n";
        fclose(in);
        fclose(out);
        return;
    }


    unsigned char* buffer = new unsigned char[bufferSize];
    size_t bytesRead;
    Node* current = root;



    while ((bytesRead = fread(buffer, 1, bufferSize, in)) > 0) {

        for (size_t i = 0; i < bytesRead; ++i) {

            for (int bit = 7; bit >= 0; --bit) {

                int bitVal = (buffer[i] >> bit) & 1; //ai helped us in this part

                current = bitVal ? current->right : current->left;

                if (!current->left && !current->right) {

                    fputc(current->ch, out);
                    current = root;

                }
            }
        }
    }

    delete[] buffer;
    freeTree(root);

    fclose(in);
    fclose(out);

    printf(" Decompression complete. Output written to %s\n", outputFile);

}

//  Cleanup  

void freeFreqList(FreqNode* head) {

    while (head) {
        FreqNode* temp = head;
        head = head->next;
        delete temp;
    }

}

void freePQ(PQNode* head) {

    while (head) {
        PQNode* temp = head;
        head = head->next;
        delete temp;
    }

}



int main(int argc, char* argv[]) {

    if (argc < 4) {
        cout << "Usage:\n"
            << "  app.exe [-b BUFFERSIZE] -c input.txt output.ece2103\n"
            << "  app.exe [-b BUFFERSIZE] -d input.ece2103 output.txt\n";
        return 1;
    }


    size_t bufferSize = 8192; // Default 8KB
    bool compress = false, decompress = false;
    const char* inputFile = nullptr;
    const char* outputFile = nullptr;



    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-b") == 0) {

            if (i + 1 >= argc) {
                cout << "Error: Missing buffer size after -b\n";
                return 1;
            }

            int kb = atoi(argv[i + 1]); //to convert string to integer

            if (kb < 1) kb = 1;
            bufferSize = static_cast<size_t>(kb) * 1024; // Convert KB to bytes
            i += 2;
        }

        else if (strcmp(argv[i], "-c") == 0)
        {
            compress = true;
            if (i + 2 >= argc) {
                cout << "Error: Missing input/output files for compression.\n";
                return 1;
            }
            inputFile = argv[i + 1];
            outputFile = argv[i + 2];
            i += 3;
        }
        else if (strcmp(argv[i], "-d") == 0) {
            decompress = true;
            if (i + 2 >= argc) {
                cout << "Error: Missing input/output files for decompression.\n";
                return 1;
            }
            inputFile = argv[i + 1];
            outputFile = argv[i + 2];
            i += 3;
        }
        else {
            cout << "Error: Unknown argument: " << argv[i] << "\n";
            return 1;
        }
    }

    if (compress && decompress) {
        cout << "Error: Cannot specify both -c and -d.\n";
        return 1;
    }
    // user chose to compress
    if (compress) {

        FreqNode* freqList = nullptr;
        readFrequencies(inputFile, freqList, bufferSize);

        PQNode* pq = nullptr;
        buildPriorityQueue(freqList, pq);
        freeFreqList(freqList);

        Node* root = buildHuffmanTree(pq);
        freePQ(pq);

        char codes[256][256] = { 0 };
        char path[256];
        generateCodes(root, codes, path, 0);

        compressFile(inputFile, outputFile, root, codes, bufferSize);

        freeTree(root);
    }
    //user chose to decompress
    else if (decompress) {

        decompressFile(inputFile, outputFile, bufferSize);
    }

    else {

        cout << "Error: Must specify -c (compress) or -d (decompress).\n";
        return 1;

    }


    return 0;

}