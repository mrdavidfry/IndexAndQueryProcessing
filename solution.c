#include "solution.h"
#include <stdio.h>
#include <stdlib.h>

#define HASH_TABLE_SIZE 128000
#define NUM_ORDER_DISCOUNTS 100
#define MAX_DISCOUNT 99
#define MAX_COUNTRY_ID 195
#define MAX_ITEM_PRICE 8191
#define MAX_MANAGER_ID 895

struct StoreNode {
  struct StoreTuple* tuple;
  struct StoreNode* next;
};

struct OrderSalesDateNode {
  int salesDate;
  struct OrderSalesDateNode *next;
};

// Stores Orders, categorized by their discount
struct OrdersHashtable {
    struct OrderSalesDateNode *table[NUM_ORDER_DISCOUNTS];
};

struct StoreTupleHashtable {
  struct StoreNode *table [HASH_TABLE_SIZE];
};

struct ItemTupleHashtable {
  struct ItemNode *table [HASH_TABLE_SIZE];
};

struct ItemNode {
  struct ItemTuple* tuple;
  struct ItemNode* next;
};

struct StoreNode* createStoreNode(struct StoreTuple *tuple);
struct StoreTupleHashtable* initialiseHashtableStores();
int computeHashMultiple(int k1, int k2);
struct StoreNode* createStoreNode(struct StoreTuple *tuple);
int findByValuesStores(struct StoreTupleHashtable *table, int pos, int managerID, int countryID);
struct ItemTupleHashtable* initialiseHashtableItems();
struct ItemNode* createItemNode(struct ItemTuple* tuple);
void insertItemIntoTable (struct ItemTupleHashtable *table, struct ItemNode *node, int pos);
void insertStoreIntoTable(struct StoreTupleHashtable *table, struct StoreNode *node, int pos);
void destroyItemTupleHashtable(struct ItemTupleHashtable* table);
void destroyStoreTupleHashtable(struct StoreTupleHashtable* table);
int findByValuesItem (struct ItemTupleHashtable *table, int pos, int salesDate, int employee,
                      int checkPrice, int price);

struct OrdersHashtable *hashTableOrders;
struct StoreTupleHashtable* hashtableStore;
struct ItemTupleHashtable* hashtableItems;

struct ItemTupleHashtable* initialiseHashtableItems() {
  struct ItemTupleHashtable *table = malloc (sizeof(struct ItemTupleHashtable));

  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    table->table[i] = NULL;
  }

  return table;
}


void destroyItemTupleHashtable(struct ItemTupleHashtable* table) {
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    for (struct ItemNode *n = table->table[i]; n != NULL;) {
      struct ItemNode *next = n->next;
      free(n);
      n = next;
    }
    table->table[i] = NULL;
  }
  free(table);
}

void destroyStoreTupleHashtable(struct StoreTupleHashtable* table) {
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    for (struct StoreNode *n = table->table[i]; n != NULL;) {
      struct StoreNode *next = n->next;
      free(n);
      n = next;
    }
    table->table[i] = NULL;
  }
  free(table);
}

struct StoreTupleHashtable* initialiseHashtableStores() {
  struct StoreTupleHashtable *table = malloc (sizeof(struct StoreTupleHashtable));
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    table->table[i] = NULL;
  }
  return table;
}

int computeHashMultiple(int k1, int k2) {
  // Cantor pairing function
  int val = ((((k1 + k2) * (k1 + k2 + 1)) / 2) + k2) % HASH_TABLE_SIZE;
  return val < 0 ? val * -1 : val;
}

void insertItemIntoTable (struct ItemTupleHashtable *table, struct ItemNode *node, int pos) {
  struct ItemNode *n = table->table[pos];
  table->table[pos] = node;
  table->table[pos]->next = n;
}


void insertStoreIntoTable(struct StoreTupleHashtable *table, struct StoreNode *node, int pos) {
  struct StoreNode *n = table->table[pos];
  table->table[pos] = node;
  table->table[pos]->next = n;
}

struct StoreNode *createStoreNode(struct StoreTuple *tuple) {
  struct StoreNode *node = malloc (sizeof(struct StoreNode));
  node->tuple = tuple;
  node->next = NULL;
  return node;
}

struct ItemNode *createItemNode(struct ItemTuple *tuple) {
  struct ItemNode *node = malloc (sizeof(struct ItemNode));
  node->tuple = tuple;
  node->next = NULL;
  return node;
}


int findByValuesStores(struct StoreTupleHashtable *table, int pos, int managerID, int countryID) {
  int count = 0;
  for (struct StoreNode *node = table->table[pos]; node != NULL; node = node->next) {
    if (node->tuple->managerID == managerID && node->tuple->countryID == countryID)
      count++;
  }
  return count;
}

int findByValuesItem (struct ItemTupleHashtable *table, int pos, int salesDate, int employee,
                      int checkPrice, int price) {
  int count = 0;
  for (struct ItemNode *node = table->table[pos]; node != NULL; node = node->next) {
    if ((checkPrice && node->tuple->price < price) || !checkPrice) {
      if (node->tuple->salesDate == salesDate && node->tuple->employee == employee)
        count++;
    }
  }
  return count;
}


int Query1(struct Database* db, int managerID, int price) {

  if (price > MAX_ITEM_PRICE || managerID > MAX_MANAGER_ID) {
          return 0;
  }

  int count = 0;

  for (size_t i = 0; i < db->ordersCardinality; i++) {
    if (db->orders[i].employeeManagerID == managerID) {
      int hash = computeHashMultiple(db->orders[i].salesDate, db->orders[i].employee);
      count += findByValuesItem(hashtableItems, hash, db->orders[i].salesDate, db->orders[i].employee, 1, price);
    }
  }
  return count;
}

int Query2(struct Database* db, int discount, int date) {

  if (discount > MAX_DISCOUNT) {
    return 0;
  }

  int count = 0;

  struct OrderSalesDateNode *relevantOrdersCurItem = hashTableOrders->table[discount];
  while (relevantOrdersCurItem != NULL) {
    for (size_t i = 0; i < db->itemsCardinality; i++) {
        int itemSalesDate = db->items[i].salesDate;
        if (relevantOrdersCurItem->salesDate >= itemSalesDate
        && relevantOrdersCurItem->salesDate <= itemSalesDate + date) {
            count++;
        }
    }
    relevantOrdersCurItem = relevantOrdersCurItem->next;
  }
  return count;
}

int Query3(struct Database* db, int countryID) {

  if (countryID > MAX_COUNTRY_ID) {
    return 0;
  }

  int count = 0;

  for (size_t i = 0; i < db->ordersCardinality; i++) {
    int hash = computeHashMultiple(db->orders[i].employeeManagerID, countryID);
    int stores = findByValuesStores(hashtableStore, hash, db->orders[i].employeeManagerID, countryID);
    if (stores>0) {
      int hash2 = computeHashMultiple(db->orders[i].salesDate, db->orders[i].employee);
      count += findByValuesItem(hashtableItems, hash2, db->orders[i].salesDate, db->orders[i].employee, 0, 0)*stores;
    }
  }

  return count;
}

void CreateIndices(struct Database* db){
  // init orders hash table:
  hashTableOrders = (struct OrdersHashtable *) malloc(sizeof(struct OrdersHashtable));
  for (int i = 0; i < NUM_ORDER_DISCOUNTS; i++) {
    hashTableOrders->table[i] = NULL;
  }

  for (size_t i = 0; i < db->ordersCardinality; i++) {
    struct OrderSalesDateNode *node = (struct OrderSalesDateNode *) malloc(sizeof(struct OrderSalesDateNode));
    node->salesDate = db->orders[i].salesDate;

    struct OrderSalesDateNode * bucketStart = hashTableOrders->table[db->orders[i].discount]; // we bucket by discount
    if (bucketStart == NULL) {
      // A node with this discount hasn't been inserted yet, create the bucket:
      node->next = NULL;
      hashTableOrders->table[db->orders[i].discount] = node;
    } else {
      // Insert item into head of bucket
      hashTableOrders->table[db->orders[i].discount] = node;
      node->next = bucketStart;
    }
  }

  hashtableStore = initialiseHashtableStores();
  hashtableItems = initialiseHashtableItems();

  for (size_t i = 0; i < db->itemsCardinality; i++) {
    struct ItemNode *node = createItemNode(&db->items[i]);
    int hash = computeHashMultiple(db->items[i].salesDate, db->items[i].employee);
    insertItemIntoTable(hashtableItems, node, hash);
  }

  for (size_t i = 0; i < db->storesCardinality; i++) {
    struct StoreNode *node = createStoreNode(&db->stores[i]);
    int hash = computeHashMultiple(db->stores[i].managerID, db->stores[i].countryID);
    insertStoreIntoTable(hashtableStore, node, hash);
  }

}


void DestroyIndices(struct Database* db) {
  (void) db;
  // free OrdersHashtable:
  for (int i = 0; i < NUM_ORDER_DISCOUNTS; i++) {
    for (struct OrderSalesDateNode *node = hashTableOrders->table[i]; node != NULL;) {
      struct OrderSalesDateNode *next = node->next;
      free(node);
      node = next;
    }
  }
  free(hashTableOrders);

  destroyItemTupleHashtable(hashtableItems);
  destroyStoreTupleHashtable(hashtableStore);
}
