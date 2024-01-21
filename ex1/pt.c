#include <stdbool.h>
#include <stddef.h>

#include "os.h"

#define LEVELS_COUNT 5

/**
 * Get the requested trie level from the VPN.
 */
inline uint16_t get_trie_level(uint64_t vpn, uint8_t level) {
    return (vpn >> (45 - 9 * level)) & 0x1FF;
}

/**
 * Receive a page table entry of any level, and return whether the valid bit is
 * set.
 */
inline bool is_entry_valid(uint64_t pte) {
    return pte & 0x1;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    uint64_t *curr_table = (uint64_t *)phys_to_virt(pt << 12);
    uint16_t curr_level_index;
    uint64_t curr_level_entry;

    for (uint8_t i = 1; i < LEVELS_COUNT; i++) {
        curr_level_index = get_trie_level(vpn, i);
        curr_level_entry = curr_table[curr_level_index];
        if (!is_entry_valid(curr_level_entry)) {
            if (ppn == NO_MAPPING) {
                return;
            }
            curr_table[curr_level_index] = (alloc_page_frame() << 12) | 0x1;
        }
        uint64_t next_table_addr = curr_table[curr_level_index] & ~0xFFF;
        curr_table = phys_to_virt(next_table_addr);
    }

    curr_level_index = get_trie_level(vpn, LEVELS_COUNT);
    if (ppn == NO_MAPPING) {
        curr_table[curr_level_index] &= 0xFFFFFFFFFFFFFFFE;
    } else {
        curr_table[curr_level_index] = (ppn << 12) + 0x1;
    }
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t *curr_table = (uint64_t *)phys_to_virt(pt << 12);
    uint16_t curr_level_index;
    uint64_t curr_level_entry;

    for (uint8_t i = 1; i <= LEVELS_COUNT; i++) {
        curr_level_index = get_trie_level(vpn, i);
        curr_level_entry = curr_table[curr_level_index];
        if (!is_entry_valid(curr_level_entry)) {
            return NO_MAPPING;
        }

        if (i < LEVELS_COUNT) {
            uint64_t next_table_addr = curr_table[curr_level_index] & ~0xFFF;
            curr_table = phys_to_virt(next_table_addr);
        }
    }

    return curr_level_entry >> 12;
}
