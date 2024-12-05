#include "include/bplus.h"
#include <list>
#include <algorithm>

namespace bplus{
    
    OPERATOR_RELOAD(index_t)
    OPERATOR_RELOAD(record_t)
    KEYCMP_OPERATOR_RELOAD

    template<class T>
    inline typename T::child_t begin(T& node){
        return node.children; 
    }
    template<class T>
    inline typename T::child_t end(T& node){
        return node.children + node.num;
    }

    inline index_t *find(internal_t& node, const key_t& key){
        return std::upper_bound(begin(node), end(node) - 1, key);
    }
    inline record_t *find(leaf_t& node, const key_t& key){
        return std::lower_bound(begin(node), end(node), key);
    }

    BPlusTree::BPlusTree(const char *p, bool empty) : fp(NULL), fp_level(0){
        memset(path, 0, sizeof(path));
        strncpy(path, p, sizeof(path));

        if(!empty && map(&meta, OFFSET_META) != 0){
            empty = true;
        }
        if(empty){
            openFile("w+");
            initEmpty();
            closeFile();
        }
    }

    template<class T>
    void BPlusTree::createNode(off_t offset, T *node, T *next){
        next -> parent = node -> parent;
        next -> next = node -> next;
        next -> prev = offset;
        node -> next = alloc(next);
        if(next -> next != 0){
            T old_next;
            map(&old_next, next -> next, SIZE_NO_CHILDREN);
            old_next.prev = node -> next;
            unmap(&old_next, next -> next, SIZE_NO_CHILDREN);
        }
        unmap(&meta, OFFSET_META);
    }

    void BPlusTree::changeParent(off_t parent, const key_t& oldk, const key_t& newk){
        internal_t node;
        map(&node, parent);

        index_t *w = find(node, oldk);

        w -> key = newk;
        unmap(&node, parent);
        if(w == node.children + node.num - 1){
            changeParent(node.parent, oldk, newk);
        }
    }

    void BPlusTree::resetParent(index_t *begin, index_t *end, off_t parent){
        internal_t node;
        for(; begin != end; begin++){
            map(&node, begin->child);
            node.parent = parent;
            unmap(&node, begin->child, SIZE_NO_CHILDREN);
        }
    }

    off_t BPlusTree::searchIndex(const key_t& key) const{
        off_t index = meta.root_offset;
        int height = meta.height;
        while(height > 1){
            internal_t node;
            map(&node, index);
            index_t *i = std::upper_bound(begin(node), end(node) - 1, key);
            index = i -> child;
            height--;
        }
        return index;
    }

    off_t BPlusTree::searchLeaf(off_t index, const key_t& key) const{
        internal_t node;
        map(&node, index);

        index_t *i = std::upper_bound(begin(node), end(node) - 1, key);
        return i -> child;
    }

    int BPlusTree::searchRange(key_t *left, const key_t& right, value_t *values, size_t max, bool *next) const{
        if(left == NULL || right < *left){
            return -1;
        }
        off_t off_left = searchLeaf(*left);
        off_t off_right = searchLeaf(right);
        off_t off = off_left;
        size_t i = 0;
        record_t *b_rec, *e_rec;

        leaf_t leaf;
        while(off != off_right && off != 0 && i < max){
            map(&leaf, off);

            if(off_left == off){
                b_rec = find(leaf, *left);
            }else{
                b_rec = begin(leaf);
            }
            e_rec = leaf.children + leaf.num;
            for(; b_rec != e_rec && i < max; b_rec++, i++){
                values[i] = b_rec -> value;
            }

            off = leaf.next;
        }

        if(i < max){
            map(&leaf, off_right);

            b_rec = find(leaf, *left);
            e_rec = std::upper_bound(begin(leaf), end(leaf), right);
            for(; b_rec != e_rec && i < max; b_rec++, i++){
                values[i] = b_rec -> value;
            }
        }

        if(next != NULL){
            if(i == max && b_rec != e_rec){
                *next = true;
                *left = b_rec -> key;
            }else{
                *next = false;
            }
        }

        return i;
    }
    
    int BPlusTree::search(const key_t& key, value_t *value) const{
        leaf_t leaf;
        map(&leaf, searchLeaf(key));

        record_t *record = find(leaf, key);
        if(record != leaf.children + leaf.num){
            *value = record -> value;
            return keyCmp(record -> key, key);
        }else{
            return -1;
        }
    }

    void BPlusTree::insertRecNoSplit(leaf_t *leaf, const key_t& key, const value_t& value){
        record_t *where = std::upper_bound(begin(*leaf), end(*leaf), key);
        std::copy_backward(where, end(*leaf), end(*leaf) + 1);
        where -> key = key;
        where -> value = value;
        leaf -> num++;
    }

    void BPlusTree::insertKeyNoSplit(internal_t& node, const key_t& key, off_t value){
        index_t *where = std::upper_bound(begin(node), end(node) - 1, key);
        std::copy_backward(where, end(node), end(node) + 1);
        where -> key = key;
        where -> child = (where + 1) -> child;
        (where + 1) -> child = value;
        node.num++;
    }

    void BPlusTree::insertKey(off_t offset, const key_t& key, off_t old, off_t after){
        if(offset == 0){  // create root
            internal_t root;
            root.next = root.prev = root.parent = 0;
            meta.root_offset = alloc(&root);
            meta.height++;

            root.num = 2;
            root.children[0].key = key;
            root.children[0].child = old;
            root.children[1].child = after;

            unmap(&meta, OFFSET_META);
            unmap(&root, meta.root_offset);

            resetParent(begin(root), end(root), meta.root_offset);
            return;
        }

        internal_t node;
        map(&node, offset);

        if(node.num == meta.order){   // full split
            // find split point
            size_t point = (node.num - 1) / 2;
            bool place_right = node.children[point].key < key;
            if(place_right) point++;
            if(place_right && key < node.children[point].key) point--;

            key_t mid = node.children[point].key;

            internal_t new_node;
            createNode(offset, &node, &new_node);

            // split
            std::copy(begin(node) + point + 1, end(node), begin(new_node));
            new_node.num = node.num - point - 1;
            node.num = point + 1;

            if(place_right){
                insertKeyNoSplit(new_node, key, after);
            }else{
                insertKeyNoSplit(node, key, after);
            }

            unmap(&node, offset);
            unmap(&new_node, node.next);

            resetParent(begin(new_node), end(new_node), node.next);
            insertKey(node.parent, mid, offset, node.next);
        }else{
            insertKeyNoSplit(node, key, after);
            unmap(&node, offset);
        }
    }

    int BPlusTree::insert(const key_t& key, value_t value){
        off_t parent = searchIndex(key);
        off_t offset = searchLeaf(parent, key);
        leaf_t leaf;
        map(&leaf, offset);

        if(std::binary_search(begin(leaf), end(leaf), key))
            return 1;   //find same, return

        if(leaf.num == meta.order){
            // full, split
            leaf_t new_leaf;
            createNode(offset, &leaf, &new_leaf);

            // find split point
            size_t point = leaf.num / 2;
            bool place_right = leaf.children[point].key < key;
            if(place_right)
                point++;

            // split
            std::copy(leaf.children + point, leaf.children + leaf.num, new_leaf.children);
            new_leaf.num = leaf.num - point;
            leaf.num = point;

            if(place_right){
                insertRecNoSplit(&new_leaf, key, value);
            }else{
                insertRecNoSplit(&leaf, key, value);
            }

            unmap(&leaf, offset);
            unmap(&new_leaf, leaf.next);
            insertKey(parent, new_leaf.children[0].key, offset, leaf.next);
        }else{
            insertRecNoSplit(&leaf, key, value);
            unmap(&leaf, offset);
        }
        return 0;
    }

    int BPlusTree::update(const key_t& key, value_t value){
        off_t offset = searchLeaf(key);
        leaf_t leaf;
        map(&leaf, offset);

        record_t *record = find(leaf, key);
        if(record != leaf.children + leaf.num){
            if(key == record -> key){
                record->value = value;
                unmap(&leaf, offset);
                return 0;
            }else{
                return 1;
            }
        }else{
            return -1;  // not found
        }
    }

    bool BPlusTree::borrowKey(bool from_right, internal_t& borrower, off_t offset){
        off_t lender_off = from_right ? borrower.next : borrower.prev;
        internal_t lender;
        map(&lender, lender_off);

        if(lender.num != meta.order / 2){
            internal_t::child_t to_lend, to_place;
            internal_t parent;
            if(from_right){
                to_lend = begin(lender);
                to_place = end(borrower);
                map(&parent, borrower.parent);
                internal_t::child_t where = std::lower_bound(begin(parent), end(parent) - 1, (end(borrower)-1) -> key);
                where -> key = to_lend -> key;
                unmap(&parent, borrower.parent);
            } else {
                to_lend = end(lender) - 1;
                to_place = begin(borrower);
                map(&parent, lender.parent);
                internal_t::child_t where = find(parent, begin(lender)->key);
                to_place->key = where->key;
                where->key = (to_lend - 1) -> key;
                unmap(&parent, lender.parent);
            }
            std::copy_backward(to_place, end(borrower), end(borrower) + 1);
            *to_place = *to_lend;
            borrower.num++;

            resetParent(to_lend, to_lend + 1, offset);
            std::copy(to_lend + 1, end(lender), to_lend);
            lender.num--;
            unmap(&lender, lender_off);
            return true;
        }

        return false;
    }

    bool BPlusTree::borrowKey(bool from_right, leaf_t& borrower){
        off_t lender_off = from_right ? borrower.next : borrower.prev;
        leaf_t lender;
        map(&lender, lender_off);

        if(lender.num != meta.order / 2){
            typename leaf_t::child_t to_lend;
            typename leaf_t::child_t to_place;

            if(from_right){
                to_lend = begin(lender);
                to_place = end(borrower);
                changeParent(borrower.parent, begin(borrower) -> key, lender.children[1].key);
            } else {
                to_lend = end(lender) - 1;
                to_place = begin(borrower);
                changeParent(lender.parent, begin(lender) -> key, to_lend -> key);
            }

            std::copy_backward(to_place, end(borrower), end(borrower) + 1);
            *to_place = *to_lend;
            borrower.num++;

            std::copy(to_lend + 1, end(lender), to_lend);
            lender.num--;
            unmap(&lender, lender_off);
            return true;
        }

        return false;
    }

    void BPlusTree::mergeLeaf(leaf_t *left, leaf_t *right){
        std::copy(begin(*right), end(*right), end(*left));
        left -> num += right -> num;
    }

    void BPlusTree::mergeKey(index_t *where, internal_t& node, internal_t& next){
        std::copy(begin(next), end(next), end(node));
        node.num += next.num;
        removeNode(&node, &next);
    }

    template<class T>
    void BPlusTree::removeNode(T *prev, T *node){
        unalloc(node, prev -> next);
        prev -> next = node -> next;
        if(node -> next != 0){
            T next;
            map(&next, node -> next, SIZE_NO_CHILDREN);
            next.prev = node -> prev;
            unmap(&next, node -> next, SIZE_NO_CHILDREN);
        }
        unmap(&meta, OFFSET_META);
    }

    void BPlusTree::removeIndex(off_t offset, internal_t& node, const key_t& key)
    {
        size_t min = meta.root_offset == offset ? 1 : meta.order / 2;
        key_t index_key = begin(node) -> key;
        index_t *to_delete = find(node, key);
        if(to_delete != end(node)){
            (to_delete + 1) -> child = to_delete -> child;
            std::copy(to_delete + 1, end(node), to_delete);
        }
        node.num--;

        if(node.num == 1 && meta.root_offset == offset && meta.internal_num != 1){
            unalloc(&node, meta.root_offset);
            meta.height--;
            meta.root_offset = node.children[0].child;
            unmap(&meta, OFFSET_META);
            return;
        }   // delete root, decrease height, reset root

        if(node.num < min){
            internal_t parent;
            map(&parent, node.parent);
            bool borrowed = false;
            if(offset != begin(parent) -> child)
                borrowed = borrowKey(false, node, offset);
            if(!borrowed && offset != (end(parent) - 1) -> child)
                borrowed = borrowKey(true, node, offset);

            if(!borrowed){
                if(offset == (end(parent) - 1)->child){
                    internal_t prev;
                    map(&prev, node.prev);

                    index_t *where = find(parent, begin(prev) -> key);
                    resetParent(begin(node), end(node), node.prev);
                    mergeKey(where, prev, node);
                    unmap(&prev, node.prev);
                }else{
                    internal_t next;
                    map(&next, node.next);

                    index_t *where = find(parent, index_key);
                    resetParent(begin(next), end(next), offset);
                    mergeKey(where, node, next);
                    unmap(&node, offset);
                }
                removeIndex(node.parent, parent, index_key);
            }else{
                unmap(&node, offset);
            }
        }else{
            unmap(&node, offset);
        }
    }

    int BPlusTree::remove(const key_t& key){
        internal_t parent;
        leaf_t leaf;

        off_t parent_off = searchIndex(key);
        map(&parent, parent_off);

        index_t *where = find(parent, key);
        off_t offset = where->child;
        map(&leaf, offset);

        if(!std::binary_search(begin(leaf), end(leaf), key))
            return -1;  // cannot find

        size_t min = meta.leaf_num == 1 ? 0 : meta.order / 2;
        record_t *to_delete = find(leaf, key);
        std::copy(to_delete + 1, end(leaf), to_delete);
        leaf.num--;

        if(leaf.num < min){
            bool borrowed = false;
            if(leaf.prev != 0)
                borrowed = borrowKey(false, leaf);
            if(!borrowed && leaf.next != 0)
                borrowed = borrowKey(true, leaf);
            if(!borrowed){
                key_t index_key;
                if(where == end(parent) - 1){
                    leaf_t prev;
                    map(&prev, leaf.prev);
                    index_key = begin(prev)->key;

                    mergeLeaf(&prev, &leaf);
                    removeNode(&prev, &leaf);
                    unmap(&prev, leaf.prev);
                }else{
                    leaf_t next;
                    map(&next, leaf.next);
                    index_key = begin(leaf)->key;

                    mergeLeaf(&leaf, &next);
                    removeNode(&leaf, &next);
                    unmap(&leaf, offset);
                }

                removeIndex(parent_off, parent, index_key);
            }else{
                unmap(&leaf, offset);
            }
        } else {
            unmap(&leaf, offset);
        }

        return 0;
    }

    void BPlusTree::initEmpty(){
        // init meta
        memset(&meta, 0, sizeof(meta_t));
        meta.order = BPLUS_ORDER;
        meta.value_size = sizeof(value_t);
        meta.key_size = sizeof(key_t);
        meta.height = 1;
        meta.slot = OFFSET_BLOCK;

        // init root node
        internal_t root;
        root.parent = 0;
        root.prev = root.next = 0;
        meta.root_offset = alloc(&root);

        // init leaf
        leaf_t leaf;
        leaf.next = leaf.prev = 0;
        leaf.parent = meta.root_offset;
        root.children[0].child = alloc(&leaf);
        meta.leaf_offset = root.children[0].child;

        // unmap to disk
        unmap(&meta, OFFSET_META);
        unmap(&root, meta.root_offset);
        unmap(&leaf, root.children[0].child);
    }

}
