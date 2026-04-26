#include "MMU.h"
#include <iostream>
using namespace std;

// Demo function for MMU
void MMU_Demo() {
    MMU* memory = MMU::Get_Instance();

    cout << "\n=== MMU DEMO START ===\n";

    // Allocate memory
    int h1 = memory->Alloc(50);
    int h2 = memory->Alloc(100);

    cout << "Handles: " << h1 << ", " << h2 << endl;

    // Write to memory
    memory->Write(h1, 'A');
    memory->Write(h1, 'B');

    // Read from memory
    int val = memory->Read(h1);
    cout << "Read: " << (char)val << endl;

    // Dump memory blocks
    cout << "\nMemory Blocks:\n";
    cout << memory->Dump_Blocks() << endl;

    // Free memory
    memory->Free(h1);

    cout << "\nAfter Free:\n";
    cout << memory->Dump_Blocks() << endl;

    // Failure case
    int fail = memory->Alloc(5000);
    cout << "\nFail test: " << fail << endl;

    cout << "=== MMU DEMO END ===\n";
}
