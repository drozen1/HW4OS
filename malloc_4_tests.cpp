#include "malloc_4.cpp"

#include <assert.h>
#include <iostream>
#include <stdio.h>

using std::cout;

void assert_alignment(size_t num) {
    assert(((num + 7) & (-8)) == num);
}

int main(int argc, char* argv[]) {
    cout << "Starting tests!\n";
    void *ptr1, *ptr2, *ptr3, *ptr4;

    // The compiler actually aligns structs automatically, but let's check it anyway
    assert_alignment(_size_meta_data());

    ptr1 = smalloc(7); // 8
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 8);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_meta_data_bytes() == _size_meta_data() * 1);

    ptr2 = smalloc(17); // 24
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 32);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_meta_data_bytes() == _size_meta_data() * 2);

    sfree(ptr1);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 32);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 8);
    assert(_num_meta_data_bytes() == _size_meta_data() * 2);

    sfree(ptr2); // should merge with ptr1
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 64);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 64);
    assert(_num_meta_data_bytes() == _size_meta_data() * 1);

    ptr1 = smalloc(500); // 504
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 504);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_meta_data_bytes() == _size_meta_data() * 1);

    ptr2 = smalloc(117); // 120
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 624);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_meta_data_bytes() == _size_meta_data() * 2);

    sfree(ptr1);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 624);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 504);
    assert(_num_meta_data_bytes() == _size_meta_data() * 2);

    // split
    ptr1 = smalloc(10); // 16
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 592);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 456);
    assert(_num_meta_data_bytes() == _size_meta_data() * 3);
    assert(ptr1 < ptr2);

    sfree(ptr1); // should merge
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 624);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 504);
    assert(_num_meta_data_bytes() == _size_meta_data() * 2);

    // split
    ptr1 = smalloc(10); // 16
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 592);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 456);
    assert(_num_meta_data_bytes() == _size_meta_data() * 3);
    assert(ptr1 < ptr2);

    ptr3 = smalloc(452); // 456
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 592);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_meta_data_bytes() == _size_meta_data() * 3);
    assert(ptr3 < ptr2);

    sfree(ptr1);
    sfree(ptr2);
    sfree(ptr3); // should merge
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 656);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 656);
    assert(_num_meta_data_bytes() == _size_meta_data() * 1);

    ptr1 = smalloc(318); // 320
    ptr2 = smalloc(319); // 320
    ptr3 = smalloc(320); // 320
    sfree(ptr2);
    ptr1 = srealloc(ptr1, 317); // 320 // 1a
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 960);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 320);
    assert(_num_meta_data_bytes() == _size_meta_data() * 3);

    ptr1 = srealloc(ptr1, 324); // 328 // 1c
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 960);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 312);
    assert(_num_meta_data_bytes() == _size_meta_data() * 3);

    ptr3 = srealloc(ptr3, 323); // 328 // 1b
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 960);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 304);
    assert(_num_meta_data_bytes() == _size_meta_data() * 3);

    ptr2 = smalloc(290); // 296
    assert(ptr2 > ptr3 && ptr3 > ptr1);
    sfree(ptr1);
    sfree(ptr2);
    ptr3 = srealloc(ptr3, 900); // 904 // 1d
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 1024);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_meta_data_bytes() == _size_meta_data() * 1);

    sfree(ptr3);

    ptr1 = smalloc(593); // 600
    ptr2 = smalloc(394); // 400
    ptr3 = smalloc(395); // 400
    ptr4 = smalloc(396); // 400
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == 1800);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_meta_data_bytes() == _size_meta_data() * 4);

    sfree(ptr1);
    ptr3 = srealloc(ptr3, 594); // 600 // 1e
    assert(ptr3 < ptr2);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == 1800);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 400);
    assert(_num_meta_data_bytes() == _size_meta_data() * 4);

    ptr2 = srealloc(ptr2, 999); // 1000 // 1f
    assert(ptr4 < ptr2);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == 2832);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 832);
    assert(_num_meta_data_bytes() == _size_meta_data() * 4);

    cout << "Tests Ended Successfully!\n";
    return 0;
}