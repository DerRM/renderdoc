#include "sfo.h"
#include "logging.h"

u32 sfo_header = 0x46535000; //Big endian

void getTitle(u8 *sfo, u32 sfo_offset, u32 sfo_size, uint8_t* title, uint32_t* length)
{
    u32 sfo_vals = le32(sfo + sfo_offset + 0x08) + sfo_offset;
    u32 sfo_param = le32(sfo + sfo_offset + 0x0c) + sfo_offset;
    u32 sfo_type;
    u32 sfo_n_params = le32(sfo + sfo_offset + 0x10);
    u32 sfo_val_ptr, sfo_val_size, sfo_param_ptr, sfo_param_size;
    u32 i, k;

    for (i = 0x0; i < sfo_n_params; i += 0x1)
    {
        char value[0x20];
        if (i != sfo_n_params - 1)
            sfo_val_size = le16(sfo + sfo_offset + (0x24 + (0x10 * i))) - le16(sfo + sfo_offset + (0x14 + (i * 0x10)));
        else
            sfo_val_size = 0x8;
        sfo_val_ptr = sfo_vals + le16(sfo + sfo_offset + (0x14 + (0x10 * i)));
        sfo_param_size = le32(sfo + sfo_offset + (0x1c + (0x10 * i)));
        sfo_param_ptr = le32(sfo + sfo_offset + (0x20 + (0x10 * i))) + sfo_param;
        sfo_type = le16(sfo + sfo_offset + (0x16 + (0x10 * i)));

        for (k = 0; k < (unsigned int)(u32)sfo_val_size; k++)
            value[k] = be8(sfo + (sfo_val_ptr + k));
        value[sfo_val_size] = '\0';

        if (strcmp("TITLE", value) != 0) 
            continue;

        if (sfo_param_size == 0x4)
        {
            if (sfo_type == UTF8)
            {
                for (k = 0x0; k < sfo_param_size; k += 0x1)
                    title[k] = le8(sfo + sfo_param_ptr + k);
                *length = sfo_param_size;
            }
        }
        else if (sfo_param_size > 0x4 && sfo_param_size <= 0x8)
        {
            if (sfo_type == UTF8)
            {
                for (k = 0x0; k < sfo_param_size; k += 0x1)
                    title[k] = le8(sfo + sfo_param_ptr + k);
                *length = sfo_param_size;
            }
        }
        else if (sfo_param_size > 0x8 && sfo_param_size <= 0x16)
        {
            if (sfo_type == UTF8)
            {
                for (k = 0x0; k < sfo_param_size; k += 0x1)
                    title[k] = le8(sfo + sfo_param_ptr + k);
                *length = sfo_param_size;
            }
        }
        else if (sfo_param_size > 0x16 && sfo_param_size <= 0x30)
        {
            if (sfo_type == UTF8)
            {
                for (k = 0x0; k < 0x30; k += 0x1)
                    title[k] = le8(sfo + sfo_param_ptr + k);
                *length = sfo_param_size;
            }
        }
        else if (sfo_param_size >= 0x32)
        {
            if (sfo_type == UTF8)
            {
                for (k = 0x0; k < sfo_param_size; k++)
                    title[k] = le8(sfo + sfo_param_ptr + k);
                *length = sfo_param_size;
            }
        }
        else
        {
        }
    }
}

void analizeSFO(u8 *sfo, u32 sfo_offset, u32 sfo_size)
{
    u32 sfo_vals = le32(sfo + sfo_offset + 0x08) + sfo_offset;
    u32 sfo_param = le32(sfo + sfo_offset + 0x0c) + sfo_offset;
    u32 sfo_type;
    u32 sfo_n_params = le32(sfo + sfo_offset + 0x10);
    u32 sfo_val_ptr, sfo_val_size, sfo_param_ptr, sfo_param_size;
    u64 param32;
    u32 i, k;

    LOG("[SFO HDR]     0x%08x\n", (unsigned int)le32(sfo + sfo_offset));
    LOG("[SFO Version] 0x%08x\n", (unsigned int)le32(sfo + sfo_offset + 0x4));
    LOG("[SFO N]	      %u Value(s)\n", sfo_n_params);
    LOG("[SFO Offset]  0x%08x\n", sfo_offset);
    LOG("[SFO Size]    0x%08x\n", sfo_size);
    LOG("[SFO Values]  0x%08x\n", sfo_vals);
    LOG("[SFO Params]  0x%08x\n", sfo_param);
    LOG("[ SFO ]\n");
    for (i = 0x0; i < sfo_n_params; i += 0x1)
    {
        char value[0x20];
        char param[0x200];
        if (i != sfo_n_params - 1)
            sfo_val_size = le16(sfo + sfo_offset + (0x24 + (0x10 * i))) - le16(sfo + sfo_offset + (0x14 + (i * 0x10)));
        else
            sfo_val_size = 0x8;
        sfo_val_ptr = sfo_vals + le16(sfo + sfo_offset + (0x14 + (0x10 * i)));
        sfo_param_size = le32(sfo + sfo_offset + (0x1c + (0x10 * i)));
        sfo_param_ptr = le32(sfo + sfo_offset + (0x20 + (0x10 * i))) + sfo_param;
        sfo_type = le16(sfo + sfo_offset + (0x16 + (0x10 * i)));

        for (k = 0; k < (unsigned int)(u32)sfo_val_size; k++)
            value[k] = be8(sfo + (sfo_val_ptr + k));
        LOG("[ %3i ] %20s", i + 1, value);
        if (sfo_param_size == 0x4)
        {
            if (sfo_type != UTF8)
            {
                param32 = le32(sfo + sfo_param_ptr);
                LOG(" | Param: 0x%x\n", (unsigned int)param32);
            }
            else
            {
                for (k = 0x0; k < sfo_param_size; k += 0x1)
                    param[k] = le8(sfo + sfo_param_ptr + k);
                param[sfo_param_size] = '\0';
                LOG(" | Param: %s\n", param);
            }
        }
        else if (sfo_param_size > 0x4 && sfo_param_size <= 0x8)
        {
            if (sfo_type != UTF8)
            {
                param32 = be32(sfo + sfo_param_ptr);
                LOG(" | Param: 0x%x\n", (unsigned int)param32);
            }
            else
            {
                for (k = 0x0; k < sfo_param_size; k += 0x1)
                    param[k] = le8(sfo + sfo_param_ptr + k);
                param[sfo_param_size] = '\0';
                LOG(" | Param: %s\n", param);
            }
        }
        else if (sfo_param_size > 0x8 && sfo_param_size <= 0x16)
        {
            if (sfo_type != UTF8)
            {
                param32 = be64(sfo + sfo_param_ptr);
                LOG(" | Param: 0x%x\n", (unsigned int)param32);
            }
            else
            {
                for (k = 0x0; k < sfo_param_size; k += 0x1)
                    param[k] = le8(sfo + sfo_param_ptr + k);
                param[sfo_param_size] = '\0';
                LOG(" | Param: %s\n", param);
            }
        }
        else if (sfo_param_size > 0x16 && sfo_param_size <= 0x30)
        {
            if (sfo_type != UTF8)
            {
                param32 = be64(sfo + sfo_param_ptr);
                LOG(" | Param: 0x%x\n", (unsigned int)param32);
            }
            else
            {
                for (k = 0x0; k < 0x30; k += 0x1)
                    param[k] = le8(sfo + sfo_param_ptr + k);
                param[sfo_param_size] = '\0';
                LOG(" | Param: %s\n", param);
            }
        }
        else if (sfo_param_size >= 0x32)
        {
            if (sfo_type != UTF8)
            {
                param32 = be64(sfo + sfo_param_ptr);
                LOG(" | Param: 0x%x\n", (unsigned int)param32);
            }
            else
            {
                for (k = 0x0; k < sfo_param_size; k++)
                    param[k] = le8(sfo + sfo_param_ptr + k);
                param[sfo_param_size] = '\0';
                LOG(" | Param: %s\n", param);
            }
        }
        else
        {
        }
    }
}
