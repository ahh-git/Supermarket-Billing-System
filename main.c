#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
// Structure for Binary Search Tree (BST) Node
struct node {
    int id;
    char name[30];
    float price;
    int stock;
    struct node *left, *right;
};
 
// Structure for Linked List Node (Shopping Cart)
struct cartNode {
    int id;
    char name[30];
    int qty;
    float price;
    float total;
    struct cartNode *next;
};
 
// --- Stack Implementation for Undo Feature ---
int undoStack[100];
int top = -1;
 
// Function to push an item ID to the stack
void pushStack(int id) {
    if (top < 99) {
        top++;
        undoStack[top] = id;
    }
}
 
// Function to pop the last added item ID from the stack
int popStack() {
    if (top >= 0) {
        int id = undoStack[top];
        top--;
        return id;
    }
    return -1; // Indicates the stack is empty
}
 
// --- Binary Search Tree (BST) Functions ---
 
// Insert a new product into the BST
struct node* insertNode(struct node* root, int id, char* name, float price, int stock) {
    if (root == NULL) {
        struct node* temp = (struct node*)malloc(sizeof(struct node));
        temp->id = id;
        strcpy(temp->name, name);
        temp->price = price;
        temp->stock = stock;
        temp->left = temp->right = NULL;
        return temp;
    }
    // Traverse left or right based on the ID
    if (id < root->id) 
        root->left = insertNode(root->left, id, name, price, stock);
    else if (id > root->id) 
        root->right = insertNode(root->right, id, name, price, stock);
    
    return root;
}
 
// Search for a product by its ID ($O(\log n)$ Time Complexity)
struct node* searchNode(struct node* root, int id) {
    if (root == NULL || root->id == id) 
        return root;
    if (id < root->id) 
        return searchNode(root->left, id);
    return searchNode(root->right, id);
}
 
// Helper function to find the minimum value node (for BST Deletion)
struct node* findMin(struct node* root) {
    while (root->left != NULL) root = root->left;
    return root;
}
 
// Delete a product from the BST and maintain tree balance
struct node* deleteNode(struct node* root, int id) {
    if (root == NULL) return root;
 
    if (id < root->id) 
        root->left = deleteNode(root->left, id);
    else if (id > root->id) 
        root->right = deleteNode(root->right, id);
    else {
        // Case 1 & 2: Node with 0 or 1 child
        if (root->left == NULL) {
            struct node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            struct node* temp = root->left;
            free(root);
            return temp;
        }
        // Case 3: Node with 2 children (Find inorder successor)
        struct node* temp = findMin(root->right);
        root->id = temp->id;
        strcpy(root->name, temp->name);
        root->price = temp->price;
        root->stock = temp->stock;
        // Delete the inorder successor
        root->right = deleteNode(root->right, temp->id);
    }
    return root;
}
 
// In-order traversal to print inventory sorted by ID
void printInorder(struct node* root) {
    if (root != NULL) {
        printInorder(root->left);
        printf("%d\t%.2f\t%d\t%s\n", root->id, root->price, root->stock, root->name);
        printInorder(root->right);
    }
}
 
// Substring search to find products by name
void searchByName(struct node* root, char* key, int* count) {
    if (root != NULL) {
        searchByName(root->left, key, count);
        if (strstr(root->name, key) != NULL) {
            printf("%d\t%.2f\t%d\t%s\n", root->id, root->price, root->stock, root->name);
            (*count)++;
        }
        searchByName(root->right, key, count);
    }
}
 
// --- File Handling Operations (Memory) ---
 
// Helper function to write tree nodes to the file
void writeToFile(struct node* root, FILE* fp) {
    if (root != NULL) {
        writeToFile(root->left, fp);
        fprintf(fp, "%d %s %f %d\n", root->id, root->name, root->price, root->stock);
        writeToFile(root->right, fp);
    }
}
 
// Save the entire BST to the hard drive
void saveData(struct node* root) {
    FILE* fp = fopen("data.txt", "w");
    if (fp != NULL) {
        writeToFile(root, fp);
        fclose(fp);
    }
}
 
// Load database from the hard drive at startup
struct node* loadData(struct node* root) {
    FILE* fp = fopen("data.txt", "r");
    if (fp == NULL) {
        // Create default items if the file does not exist
        root = insertNode(root, 101, "Apple", 25.0, 50);
        root = insertNode(root, 102, "Bread", 45.0, 20);
        root = insertNode(root, 103, "Milk", 80.0, 30);
        saveData(root);
        return root;
    }
    
    int id, stock;
    char name[30];
    float price;
    // Read data line by line
    while (fscanf(fp, "%d %29s %f %d", &id, name, &price, &stock) != EOF) {
        root = insertNode(root, id, name, price, stock);
    }
    fclose(fp);
    return root;
}
 
// --- Cart Operations (Linked List) ---
 
// Add a new node to the end of the linked list
struct cartNode* addCart(struct cartNode* head, int id, char* name, int qty, float price) {
    struct cartNode* temp = (struct cartNode*)malloc(sizeof(struct cartNode));
    temp->id = id;
    strcpy(temp->name, name);
    temp->qty = qty;
    temp->price = price;
    temp->total = price * qty;
    temp->next = NULL;
 
    if (head == NULL) return temp;
    
    struct cartNode* ptr = head;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = temp;
    return head;
}
 
// Remove a specific node from the linked list and restore stock
struct cartNode* removeCart(struct cartNode* head, struct node* root, int id) {
    if (head == NULL) return NULL;
    
    struct cartNode *temp = head, *prev = NULL;
    
    // If the item to be removed is the head node
    if (temp != NULL && temp->id == id) {
        head = temp->next;
        struct node* item = searchNode(root, id);
        if(item != NULL) item->stock += temp->qty;
        free(temp);
        return head;
    }
    
    // Search for the node in the linked list
    while (temp != NULL && temp->id != id) {
        prev = temp;
        temp = temp->next;
    }
    
    if (temp == NULL) return head;
    
    // Unlink the node and free memory
    prev->next = temp->next;
    struct node* item = searchNode(root, id);
    if(item != NULL) item->stock += temp->qty;
    free(temp);
    return head;
}
 
// Function to clear the entire linked list and free memory
struct cartNode* clearCart(struct cartNode* head, struct node* root) {
    while (head != NULL) {
        struct cartNode* temp = head;
        head = head->next;
        struct node* item = searchNode(root, temp->id);
        if (item != NULL) item->stock += temp->qty; // Restore stock
        free(temp); // Prevent memory leak
    }
    return NULL;
}
 
// Traverse and print the linked list
void printCart(struct cartNode* head) {
    if (head == NULL) {
        printf("Cart is empty right now.\n");
        return;
    }
    struct cartNode* temp = head;
    printf("ID\tName\tQty\tTotal\n");
    printf("--------------------------------\n");
    while (temp != NULL) {
        printf("%d\t%s\t%d\t%.2f\n", temp->id, temp->name, temp->qty, temp->total);
        temp = temp->next;
    }
}
 
// --- Main Execution ---
 
int main() {
    struct node* root = NULL;
    struct cartNode* cart = NULL;
    
    printf("Loading data...\n");
    root = loadData(root);
    
    int choice;
    while(1) {
        printf("\n=== DIU Super Shop ===\n");
        printf("1. View Items & Buy\n");
        printf("2. View Cart\n");
        printf("3. Remove from Cart\n");
        printf("4. Undo Last Add (Stack)\n");
        printf("5. Clear Whole Cart\n");
        printf("6. Admin Menu (Add/Update/Delete)\n");
        printf("7. Checkout & Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        if (choice == 1) {
            printf("\nID\tPrice\tStock\tName\n");
            printf("--------------------------------\n");
            printInorder(root); // Display available products
            
            int id, qty;
            printf("\nEnter ID to buy: ");
            scanf("%d", &id);
            struct node* item = searchNode(root, id);
            
            if (item != NULL) {
                printf("Enter quantity: ");
                scanf("%d", &qty);
                if (qty > 0 && qty <= item->stock) {
                    item->stock -= qty; // Deduct stock from BST
                    cart = addCart(cart, item->id, item->name, qty, item->price);
                    pushStack(item->id); // Store operation in stack for Undo feature
                    printf("Added to cart successfully.\n");
                } else {
                    printf("Not enough stock!\n");
                }
            } else {
                printf("Item not found!\n");
            }
        }
        else if (choice == 2) {
            printf("\n--- My Cart ---\n");
            printCart(cart);
        }
        else if (choice == 3) {
            printCart(cart);
            if (cart != NULL) {
                int id;
                printf("Enter ID to remove: ");
                scanf("%d", &id);
                cart = removeCart(cart, root, id);
                printf("Item removed from cart.\n");
            }
        }
        else if (choice == 4) {
            // Undo logic using Stack
            int lastAddedId = popStack();
            if (lastAddedId != -1) {
                cart = removeCart(cart, root, lastAddedId);
                printf("Undo successful! Last item removed.\n");
            } else {
                printf("Nothing to undo! Stack is empty.\n");
            }
        }
        else if (choice == 5) {
            if (cart != NULL) {
                cart = clearCart(cart, root);
                top = -1; // Reset stack when cart is cleared
                printf("Cart cleared successfully.\n");
            } else {
                printf("Cart is already empty.\n");
            }
        }
        else if (choice == 6) {
            char pass[20];
            printf("\nEnter Admin Password: ");
            // Use %[^\n]s format specifier to accept strings with spaces
            scanf(" %[^\n]s", pass);
            
            if (strcmp(pass, "912 444") == 0) {
                int adminChoice;
                printf("\n-- Admin Menu --\n");
                printf("1. Add Item\n2. Update Item\n3. Delete Item\n4. Search Item\n");
                printf("Enter choice: ");
                scanf("%d", &adminChoice);
                
                if (adminChoice == 1) {
                    int id, stock; char name[30]; float price;
                    printf("Enter ID: "); scanf("%d", &id);
                    if (searchNode(root, id) != NULL) printf("ID already exists!\n");
                    else {
                        printf("Name: "); scanf("%s", name);
                        printf("Price: "); scanf("%f", &price);
                        printf("Stock: "); scanf("%d", &stock);
                        root = insertNode(root, id, name, price, stock);
                        saveData(root); // Synchronize memory
                        printf("New item added.\n");
                    }
                }
                else if (adminChoice == 2) {
                    int id;
                    printf("Enter ID to update: "); scanf("%d", &id);
                    struct node* item = searchNode(root, id);
                    if (item != NULL) {
                        printf("New price: "); scanf("%f", &item->price);
                        printf("New stock: "); scanf("%d", &item->stock);
                        saveData(root);
                        printf("Item updated.\n");
                    } else printf("Item not found!\n");
                }
                else if (adminChoice == 3) {
                    int id;
                    printf("Enter ID to delete: "); scanf("%d", &id);
                    if (searchNode(root, id) != NULL) {
                        root = deleteNode(root, id);
                        saveData(root);
                        printf("Item deleted.\n");
                    } else printf("Item not found!\n");
                }
                else if (adminChoice == 4) {
                    char key[30]; int count = 0;
                    printf("Enter name to search: "); scanf("%s", key);
                    searchByName(root, key, &count);
                    if (count == 0) printf("No items found.\n");
                }
            } else {
                printf("\nWrong Password! Access Denied.\n");
            }
        }
        else if (choice == 7) {
            break; // Exit loop and proceed to checkout
        }
        else {
            printf("Wrong choice. Try again.\n");
        }
    }
    
    // --- Checkout and Final Calculations ---
    printf("\n\n=== Final Bill ===\n");
    if (cart == NULL) {
        printf("You didn't buy anything.\n");
    } else {
        float total = 0;
        struct cartNode* temp = cart;
        printf("Name\tQty\tTotal\n");
        printf("--------------------------------\n");
        // Calculate subtotal
        while (temp != NULL) {
            printf("%s\t%d\t%.2f\n", temp->name, temp->qty, temp->total);
            total += temp->total;
            temp = temp->next;
        }
        
        char isStudent;
        float discount = 0;
        printf("\nAre you a DIU Student? (y/n): ");
        scanf(" %c", &isStudent);
        
        // Apply DIU student discount
        if (isStudent == 'y' || isStudent == 'Y') {
            discount = total * 0.10; 
            printf("DIU Student Discount (10%%): -%.2f TK\n", discount);
        }
        
        total = total - discount;
        float vat = total * 0.15; // Calculate 15% VAT
        
        printf("--------------------------------\n");
        printf("Subtotal: %.2f TK\n", total);
        printf("VAT(15%%): %.2f TK\n", vat);
        printf("Grand Total: %.2f TK\n", total + vat);
        
        saveData(root); // Final database save
        printf("\nThank you for shopping!\n");
    }
    
    return 0; // Terminate program safely
}
