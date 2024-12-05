#ifndef BPLUS_H
#define BPLUS_H

#include <stddef.h>
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include "bplus_types.h"

namespace bplus{
    
    #define OFFSET_META 0
    #define OFFSET_BLOCK OFFSET_META + sizeof(meta_t)
    #define SIZE_NO_CHILDREN sizeof(leaf_t) - BPLUS_ORDER * sizeof(record_t)
    // node size with no children

    typedef struct{
        size_t order;
        size_t height;
        size_t key_size;
        size_t value_size;
        size_t internal_num;
        size_t leaf_num;
        off_t slot;
        off_t root_offset;
        off_t leaf_offset;
    }meta_t;

    struct index_t{
        key_t key;
        off_t child;
    };

    struct record_t{
        key_t key;
        value_t value;
    };

    struct internal_t{
        typedef index_t *child_t;
        off_t parent;
        off_t next;
        off_t prev;
        size_t num;   // children number
        index_t children[BPLUS_ORDER];
    };

    struct leaf_t{
        typedef record_t *child_t;

        off_t parent;
        off_t next;
        off_t prev;
        size_t num;
        record_t children[BPLUS_ORDER];
    };



    

    class BPlusTree{
        public:
            BPlusTree(const char *path, bool empty = false);
            int search(const key_t &key, value_t *value) const;
            int searchRange(key_t *left, const key_t &right, value_t *values, size_t max, bool *next=NULL) const;
            int remove(const key_t &key);
            int insert(const key_t &key, value_t value);
            int update(const key_t &key, value_t value);
        
        private:
            char path[512];
            meta_t meta;

            void initEmpty();
            off_t searchIndex(const key_t &key) const;
            off_t searchLeaf(off_t index, const key_t &key) const;
            off_t searchLeaf(const key_t &key) const{
                return searchLeaf(searchIndex(key), key);
            }
            void removeIndex(off_t offset, internal_t &node, const key_t &key);
            bool borrowKey(bool from_right, internal_t &borrower, off_t offset);
            bool borrowKey(bool from_right, leaf_t &borrower);
            void changeParent(off_t parent, const key_t &o, const key_t &n);

            void mergeLeaf(leaf_t *left, leaf_t *right);
            void mergeKey(index_t *where, internal_t &left, internal_t &right);

            void insertRecNoSplit(leaf_t *leaf, const key_t &key, const value_t &value);
            void insertKey(off_t offset, const key_t &key, off_t old, off_t after);
            void insertKeyNoSplit(internal_t &node, const key_t &key, off_t value);

            void resetParent(index_t *begin, index_t *end, off_t parent);

            template<class T>
            void createNode(off_t offset, T *node, T *next);

            template<class T>
            void removeNode(T *prev, T *node);

            mutable FILE *fp;
            mutable int fp_level = 0;
            void openFile(const char *mode = "rb+") const{
                if(fp_level == 0){
                    fp = fopen(path, mode);
                }
                ++fp_level;
            }
            void closeFile() const{
                if(fp_level == 1){
                    fclose(fp);
                }
                --fp_level;
            }

            off_t alloc(size_t size){
                off_t slot = meta.slot;     // available slot from meta
                meta.slot += size;      //update meta slot
                return slot;
            }
            off_t alloc(leaf_t *leaf){
                leaf -> num = 0;
                meta.leaf_num++;
                return alloc(sizeof(leaf_t));
            }
            off_t alloc(internal_t *node){
                node -> num = 1;
                meta.internal_num++;
                return alloc(sizeof(internal_t));
            }

            void unalloc(leaf_t *leaf, off_t offset){
                --meta.leaf_num;
            }
            void unalloc(internal_t *node, off_t offset){
                --meta.internal_num;
            }

            /* read block */
            int map(void *block, off_t offset, size_t size) const{
                openFile();
                fseek(fp, offset, SEEK_SET);
                size_t rd = fread(block, size, 1, fp);    // read count
                closeFile();
                return rd - 1;    
                // return 0 : pass, 
                // return -1 : error.
            }

            template<class T>
            int map(T *block, off_t offset) const{
                return map(block, offset, sizeof(T));
            }

            /* write block */
            int unmap(void *block, off_t offset, size_t size) const{
                openFile();
                fseek(fp, offset, SEEK_SET);
                size_t wd = fwrite(block, size, 1, fp);
                closeFile();
                return wd - 1;
            }

            template<class T>
            int unmap(T *block, off_t offset) const{
                return unmap(block, offset, sizeof(T));
            }
    };
    
}

#endif