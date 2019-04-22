#include <kernel/i386/segment.h>
#include <kernel/i386/register.h>
#include <kernel/i386/types.h>
#include <stddef.h>
#include <stdio.h>

static segment_descriptor gdt[GDT_SIZE];
static gdt_descriptor gdt_desc;

/* Fill a segment descriptor.  */
void
fill_segment_descriptor(segment_descriptor *desc, uint32_t base, uint32_t limit,
                        uint8_t access, uint8_t sizebits)
{
    if (limit > 0xfffff)
    {
        limit >>= 12;
        sizebits |= SZ_G;
    }
    desc->limit_low = limit & 0xffff;
    desc->base_low = base & 0xffff;
    desc->base_med = (base >> 16) & 0xff;
    desc->access = access | ACC_P;
    desc->limit_high = limit >> 16;
    desc->granularity = sizebits;
    desc->base_high = base >> 24;
}

/* Set the base address in a segment descriptor.  */
void
fill_descriptor_base(segment_descriptor *desc, uint32_t base)
{
    desc->base_low = base & 0xffff;
    desc->base_med = (base >> 16) & 0xff;
    desc->base_high = base >> 24;
}

/* Set the size in a segment descriptor.  */
void
fill_descriptor_limit(segment_descriptor *desc, uint32_t limit)
{
    if (limit > 0xfffff)
    {
        limit >>= 12;
        desc->granularity |= SZ_G;
    }
    else
        desc->granularity &= ~SZ_G;
    desc->limit_low = limit & 0xffff;
    desc->limit_high = limit >> 16;
}

/* Fill a gate with particular values.  */
void
fill_gate(x86_gate *gate, uint32_t offset, uint16_t selector,
          uint8_t access, uint8_t word_count)
{
    gate->offset_low = offset & 0xffff;
    gate->selector = selector;
    gate->word_count = word_count;
    gate->access = access | ACC_P;
    gate->offset_high = (offset >> 16) & 0xffff;
}

void
gdt_init()
{
    /* Initialize the 32-bit kernel code and data segment descriptors
       to point to the base of the kernel linear space region.  */
    fill_segment_descriptor(&gdt[KERNEL_CODE / 8],
                            0x00000000, 0xffffffff,
                            ACC_PL_K | ACC_CODE_R, SZ_32);
    fill_segment_descriptor(&gdt[KERNEL_DATA / 8],
                            0x00000000, 0xffffffff,
                            ACC_PL_K | ACC_DATA_W, SZ_32);

    /* Descriptors that direct-map all linear space.  */
    fill_segment_descriptor(&gdt[USER_CODE / 8],
                            0x00000000, 0xffffffff,
                            ACC_PL_U | ACC_CODE_R, SZ_32);
    fill_segment_descriptor(&gdt[USER_DATA / 8],
                            0x00000000, 0xffffffff,
                            ACC_PL_U | ACC_DATA_W, SZ_32);
}



void
gdt_load() {
    gdt_desc.base = (addr_t)(&gdt);
    gdt_desc.limit = GDT_SIZE * 8 - 1;

    /* Load it into the CPU.  */
    set_gdtr(&gdt_desc);
    /*
     * Reload all the segment registers from the new GDT.
     */
    set_cs(KERNEL_CODE);
    set_ds(KERNEL_DATA);
    set_es(KERNEL_DATA);
    set_ss(KERNEL_DATA);

    /*
     * Clear out the FS and GS registers by default,
     * since they're not needed for normal execution of GCC code.
     */
    set_fs(NULL_SEGMENT);
    set_gs(NULL_SEGMENT);

    printf("GDT: 0x%x CS: 0x%x DS: 0x%x ES: 0x%x SS: 0x%x FS: 0x%x GS: 0x%x \n", \
            &gdt, get_cs(), get_ds(), get_es(), get_ss(), get_fs(), get_gs());
}


