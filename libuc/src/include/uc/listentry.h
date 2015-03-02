#pragma once

typedef struct _LIST_ENTRY LIST_ENTRY, * PLIST_ENTRY;

struct _LIST_ENTRY {
	PLIST_ENTRY Flink;
	PLIST_ENTRY Blink;
};

#define CONTAINING_RECORD(address, type, field) \
    ((type *)( \
    (unsigned char *)(address) - \
    (unsigned char *)(&((type *)0)->field)))

static inline void InitializeListHead(PLIST_ENTRY ListHead) {
	ListHead->Flink = ListHead->Blink = ListHead;
}

static inline unsigned int IsListEmpty(LIST_ENTRY * ListHead) {
    return (ListHead->Flink == ListHead);
}

static inline unsigned int RemoveEntryList(PLIST_ENTRY Entry) {
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (Flink == Blink);
}

static inline PLIST_ENTRY RemoveHeadList(PLIST_ENTRY ListHead) {
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}

static inline PLIST_ENTRY RemoveTailList(PLIST_ENTRY ListHead) {
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}

static inline void InsertTailList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry) {
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}

static inline void InsertHeadList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry) {
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}

static inline void AppendTailList(PLIST_ENTRY ListHead, PLIST_ENTRY ListToAppend) {
    PLIST_ENTRY ListEnd = ListHead->Blink;

    ListHead->Blink->Flink = ListToAppend;
    ListHead->Blink = ListToAppend->Blink;
    ListToAppend->Blink->Flink = ListHead;
    ListToAppend->Blink = ListEnd;
}

static inline void InsertAfterEntryList(PLIST_ENTRY Entry, PLIST_ENTRY Toinsert) {
	Toinsert->Flink = Entry->Flink;
	Toinsert->Blink = Entry;
	Entry->Flink = Toinsert;
	Toinsert->Flink->Blink = Toinsert;
}

static inline void InsertBeforeEntryList(PLIST_ENTRY Entry, PLIST_ENTRY Toinsert) {
	Toinsert->Flink = Entry;
	Toinsert->Blink = Entry->Blink;
	Entry->Blink = Toinsert;
	Toinsert->Blink->Flink = Toinsert;
}

static inline void RAZEntryList(PLIST_ENTRY Entry) {
	Entry->Flink = 0;
	Entry->Blink = 0;
}

