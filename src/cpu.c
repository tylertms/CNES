#include "cpu.h"
#include "cart.h"
#include "nes.h"
#include <string.h>
#include <stdio.h>

void cpu_clock(_nes* nes) {

}

void cpu_reset(_nes* nes) {
    _cpu cpu = nes->cpu;
    uint16_t low = cpu_read(nes, RST_VECTOR);
    uint16_t high = cpu_read(nes, RST_VECTOR + 1);
    nes->cpu.pc = (high << 8) | low;

    nes->cpu.a = 0x00;
    nes->cpu.x = 0x00;
    nes->cpu.y = 0x00;

    nes->cpu.s = 0xFD;
    nes->cpu.p = 0x00 | _U | _I;

    nes->cpu.op_addr = 0x0000;
    nes->cpu.op_data = 0x00;

    nes->cpu.cycles = 7;
    nes->cpu.halt = 0;
}

void cpu_irq(_nes* nes) {
    if (!get_flag(nes, _I))
        return;

    push(nes, nes->cpu.pc >> 8);
    push(nes, nes->cpu.pc & 0xFF);

    set_flag(nes, _B, 0);
    set_flag(nes, _U, 1);
    set_flag(nes, _I, 1);
    push(nes, nes->cpu.p);

    uint16_t low = cpu_read(nes, IRQ_VECTOR);
    uint16_t high = cpu_read(nes, IRQ_VECTOR + 1);
    nes->cpu.pc = (high << 8) | low;
    nes->cpu.cycles = 7;
}

void cpu_nmi(_nes* nes) {
    int brk_nmi = !strcmp(nes->cpu.instr.opcode, "brk");
    if (!brk_nmi)
        nes->cpu.pc++;

    push(nes, nes->cpu.pc >> 8);
    push(nes, nes->cpu.pc & 0xFF);

    set_flag(nes, _B, brk_nmi);
    set_flag(nes, _U, 1);
    set_flag(nes, _I, 1);
    push(nes, nes->cpu.p);

    uint16_t low = cpu_read(nes, NMI_VECTOR);
    uint16_t high = cpu_read(nes, NMI_VECTOR + 1);
    nes->cpu.pc = (high << 8) | low;
    nes->cpu.cycles = 8;
}

uint8_t cpu_read(_nes* nes, uint16_t addr) {
    uint8_t data = 0x00;

    if (0x0000 <= addr && addr <= 0x1FFF) {
        data = nes->cpu.ram[addr & 0x07FF];
    } else if (0x2000 <= addr && addr <= 0x3FFF) {
        uint16_t reg_addr = 0x2000 | (addr & 0x0007);
        data = ppu_cpu_read(&nes->ppu, reg_addr);
    } else if (0x4000 <= addr && addr <= 0x4017) {
        // TODO: APU
    } else if (0x4020 <= addr && addr <= 0xFFFF) {
        return cart_read(&nes->cart, addr);
    }

    return 0x00;
}

uint8_t no_fetch(_nes* nes) {
    return strcmp(nes->cpu.instr.mode, "imp") |
        strcmp(nes->cpu.instr.mode, "acc");
}

uint8_t cpu_fetch(_nes* nes) {
    if (no_fetch(nes)) return nes->cpu.op_data;
    return cpu_read(nes, nes->cpu.op_addr);
}

void cpu_write_back(_nes* nes, uint8_t result) {
    if (no_fetch(nes)) nes->cpu.a = result & 0xFF;
    else cpu_write(nes, nes->cpu.op_addr, result & 0xFF);
}

void cpu_write(_nes* nes, uint16_t addr, uint8_t data) {
    if (0x0000 <= addr && addr <= 0x1FFF) {
        nes->cpu.ram[addr & 0x07FF] = data;
    } else if (0x2000 <= addr && addr <= 0x3FFF) {
        uint16_t reg_addr = 0x2000 | (addr & 0x0007);
        ppu_cpu_write(&nes->ppu, reg_addr, data);
    } else if (0x4000 <= addr && addr <= 0x4017) {
        // TODO: APU
    } else if (0x4020 <= addr && addr <= 0xFFFF) {
        cart_write(&nes->cart, addr, data);
    }
}

uint8_t get_flag(_nes* nes, _cpu_flag flag) {
    return !!(nes->cpu.p & flag);
}

void set_flag(_nes* nes, _cpu_flag flag, uint8_t set) {
    if (set) nes->cpu.p |= flag;
    else nes->cpu.p &= ~flag;
}

void push(_nes* nes, uint8_t data) {
    cpu_write(nes, 0x0100 + nes->cpu.s--, data);
}

uint8_t pull(_nes* nes) {
    return cpu_read(nes, 0x0100 + ++nes->cpu.s);
}

void branch(_nes* nes) {
    nes->cpu.cycles++;
    uint16_t res = nes->cpu.op_addr + nes->cpu.pc;

    if ((res & 0xFF00) != (nes->cpu.pc & 0xFF00))
        nes->cpu.cycles++;

    nes->cpu.pc = res;
}

/* address modes */

uint8_t am_acc(_nes* nes) {
    nes->cpu.op_data = nes->cpu.a;
    return 0;
}

uint8_t am_imp(_nes* nes) {
    nes->cpu.op_data = nes->cpu.a;
    return 0;
}

uint8_t am_imm(_nes* nes) {
    nes->cpu.op_addr = nes->cpu.pc++;
    return 0;
}

uint8_t am_zpg(_nes* nes) {
    nes->cpu.op_addr = cpu_read(nes, nes->cpu.pc++);
    return 0;
}

uint8_t am_zpx(_nes* nes) {
    nes->cpu.op_addr = (cpu_read(nes, nes->cpu.pc++) + nes->cpu.x) & 0xFF;
    return 0;
}

uint8_t am_zpy(_nes* nes) {
    nes->cpu.op_addr = (cpu_read(nes, nes->cpu.pc++) + nes->cpu.y) & 0xFF;
    return 0;
}

uint8_t am_abs(_nes* nes) {
    uint16_t low = cpu_read(nes, nes->cpu.pc++);
    uint16_t high = cpu_read(nes, nes->cpu.pc++);
    nes->cpu.op_addr = (high << 8) | low;
    return 0;
}

uint8_t am_abx(_nes* nes) {
    uint16_t low = cpu_read(nes, nes->cpu.pc++);
    uint16_t high = cpu_read(nes, nes->cpu.pc++);
    nes->cpu.op_addr = ((high << 8) | low) + nes->cpu.x;
    return (nes->cpu.op_addr & 0xFF00) != (high << 8);
}

uint8_t am_aby(_nes* nes) {
    uint16_t low = cpu_read(nes, nes->cpu.pc++);
    uint16_t high = cpu_read(nes, nes->cpu.pc++);
    nes->cpu.op_addr = ((high << 8) | low) + nes->cpu.y;
    return (nes->cpu.op_addr & 0xFF00) != (high << 8);
}

uint8_t am_idr(_nes* nes) {
    uint16_t p_low = cpu_read(nes, nes->cpu.pc++);
    uint16_t p_high = cpu_read(nes, nes->cpu.pc++);
    uint16_t ptr = (p_high << 8) | p_low;

    uint16_t high_addr = (p_low == 0xFF) ? (ptr & 0xFF00) : (ptr + 1);
    nes->cpu.op_addr = (cpu_read(nes, high_addr) << 8) | cpu_read(nes, ptr);
    return 0;
}

uint8_t am_idx(_nes* nes) {
    uint16_t base = cpu_read(nes, nes->cpu.pc++);
    uint16_t low = cpu_read(nes, (base + nes->cpu.x) & 0x00FF);
    uint16_t high = cpu_read(nes, (base + nes->cpu.x + 1) & 0x00FF);
    nes->cpu.op_addr = (high << 8) | low;
    return 0;
}

uint8_t am_idy(_nes* nes) {
    uint16_t base = cpu_read(nes, nes->cpu.pc++);
    uint16_t low = cpu_read(nes, base & 0x00FF);
    uint16_t high = cpu_read(nes, (base + 1) & 0x00FF);
    nes->cpu.op_addr = ((high << 8) | low) + nes->cpu.y;
    return (nes->cpu.op_addr & 0xFF00) != (high << 8);
}

uint8_t am_rel(_nes* nes) {
    nes->cpu.op_addr = cpu_read(nes, nes->cpu.pc++);
    if (nes->cpu.op_addr & 0x80) nes->cpu.op_addr |= 0xFF00;
    return 0;
}

uint8_t am____(_nes* nes) {
    return 0;
}

/* Operations */

uint8_t op_adc(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    uint16_t res = (uint16_t)nes->cpu.a + (uint16_t)memory + get_flag(nes, _C);

    uint16_t overflow = (res ^ nes->cpu.a) & (res ^ memory) & 0x80;
    set_flag(nes, _C, res > 0xFF);
    set_flag(nes, _Z, (res & 0xFF) == 0x00);
    set_flag(nes, _V, overflow);
    set_flag(nes, _N, res & 0x80);

    nes->cpu.a = res & 0xFF;
	return 1;
}

uint8_t op_and(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    nes->cpu.a &= memory;
    set_flag(nes, _Z, nes->cpu.a == 0x00);
    set_flag(nes, _N, nes->cpu.a & 0x80);
	return 1;
}

uint8_t op_asl(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    uint16_t res = (uint16_t)memory << 1;
    set_flag(nes, _C, res > 255);
    set_flag(nes, _Z, (res & 0xFF) == 0x00);
    set_flag(nes, _N, res & 0x80);
    cpu_write_back(nes, res);
	return 0;
}

uint8_t op_bcc(_nes* nes) {
    if (!get_flag(nes, _C))
        branch(nes);

	return 0;
}

uint8_t op_bcs(_nes* nes) {
    if (get_flag(nes, _C))
        branch(nes);
	return 0;
}

uint8_t op_beq(_nes* nes) {
    if (get_flag(nes, _Z))
        branch(nes);
    return 0;
}

uint8_t op_bit(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    uint8_t res = nes->cpu.a & memory;
    set_flag(nes, _Z, res == 0x00);
    set_flag(nes, _V, memory & 0x40);
    set_flag(nes, _N, memory & 0x80);
	return 0;
}

uint8_t op_bmi(_nes* nes) {
    if (get_flag(nes, _N))
        branch(nes);
	return 0;
}

uint8_t op_bne(_nes* nes) {
    if (!get_flag(nes, _Z))
        branch(nes);
	return 0;
}

uint8_t op_bpl(_nes* nes) {
    if (!get_flag(nes, _N))
        branch(nes);
	return 0;
}

uint8_t op_brk(_nes* nes) {
    cpu_nmi(nes);
    return 0;
}

uint8_t op_bvc(_nes* nes) {
    if (!get_flag(nes, _V))
        branch(nes);
	return 0;
}

uint8_t op_bvs(_nes* nes) {
    if (get_flag(nes, _V))
        branch(nes);
	return 0;
}

uint8_t op_clc(_nes* nes) {
    set_flag(nes, _C, 0);
	return 0;
}

uint8_t op_cld(_nes* nes) {
    set_flag(nes, _D, 0);
	return 0;
}

uint8_t op_cli(_nes* nes) {
    set_flag(nes, _I, 0);
	return 0;
}

uint8_t op_clv(_nes* nes) {
    set_flag(nes, _V, 0);
	return 0;
}

uint8_t op_cmp(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    set_flag(nes, _C, nes->cpu.a >= memory);
    set_flag(nes, _Z, nes->cpu.a == memory);
    set_flag(nes, _N, (nes->cpu.a - memory) & 0x80);
    return 1;
}

uint8_t op_cpx(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    set_flag(nes, _C, nes->cpu.x >= memory);
    set_flag(nes, _Z, nes->cpu.x == memory);
    set_flag(nes, _N, (nes->cpu.x - memory) & 0x80);
	return 0;
}

uint8_t op_cpy(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    set_flag(nes, _C, nes->cpu.y >= memory);
    set_flag(nes, _Z, nes->cpu.y == memory);
    set_flag(nes, _N, (nes->cpu.y - memory) & 0x80);
	return 0;
}

uint8_t op_dec(_nes* nes) {
    uint8_t memory = cpu_fetch(nes) - 1;
    cpu_write(nes, nes->cpu.op_addr, memory);
    set_flag(nes, _Z, memory == 0x00);
    set_flag(nes, _N, memory & 0x80);
	return 0;
}

uint8_t op_dex(_nes* nes) {
    nes->cpu.x--;
    set_flag(nes, _Z, nes->cpu.x == 0x00);
    set_flag(nes, _N, nes->cpu.x & 0x80);
	return 0;
}

uint8_t op_dey(_nes* nes) {
    nes->cpu.y--;
    set_flag(nes, _Z, nes->cpu.y == 0x00);
    set_flag(nes, _N, nes->cpu.y & 0x80);
	return 0;
}

uint8_t op_eor(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    nes->cpu.a ^= memory;
    set_flag(nes, _Z, nes->cpu.a == 0x00);
    set_flag(nes, _N, nes->cpu.a & 0x80);
	return 1;
}

uint8_t op_inc(_nes* nes) {
    uint8_t memory = cpu_fetch(nes) + 1;
    cpu_write(nes, nes->cpu.op_addr, memory);
    set_flag(nes, _Z, memory == 0x00);
    set_flag(nes, _N, memory & 0x80);
	return 0;
}

uint8_t op_inx(_nes* nes) {
    nes->cpu.x++;
    set_flag(nes, _Z, nes->cpu.x == 0x00);
    set_flag(nes, _N, nes->cpu.x & 0x80);
	return 0;
}

uint8_t op_iny(_nes* nes) {
    nes->cpu.y++;
    set_flag(nes, _Z, nes->cpu.y == 0x00);
    set_flag(nes, _N, nes->cpu.y & 0x80);
	return 0;
}

uint8_t op_jmp(_nes* nes) {
    nes->cpu.pc = nes->cpu.op_addr;
	return 0;
}

uint8_t op_jsr(_nes* nes) {
    nes->cpu.pc--;
    push(nes, nes->cpu.pc >> 8);
    push(nes, nes->cpu.pc & 0xFF);
    nes->cpu.pc = nes->cpu.op_addr;
	return 0;
}

uint8_t op_lda(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    nes->cpu.a = memory;
    set_flag(nes, _Z, memory == 0x00);
    set_flag(nes, _N, memory & 0x80);
	return 1;
}

uint8_t op_ldx(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    nes->cpu.x = memory;
    set_flag(nes, _Z, memory == 0x00);
    set_flag(nes, _N, memory & 0x80);
	return 1;
}

uint8_t op_ldy(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    nes->cpu.y = memory;
    set_flag(nes, _Z, memory == 0x00);
    set_flag(nes, _N, memory & 0x80);
	return 1;
}

uint8_t op_lsr(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    uint8_t res = memory >> 1;

    set_flag(nes, _C, memory & 0x01);
    set_flag(nes, _Z, res == 0x00);
    set_flag(nes, _N, 0);

    cpu_write_back(nes, res);
	return 0;
}

uint8_t op_nop(_nes* nes) {
	return 0;
}

uint8_t op_ora(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    nes->cpu.a |= memory;
    set_flag(nes, _Z, nes->cpu.a == 0x00);
    set_flag(nes, _N, nes->cpu.a & 0x80);
	return 1;
}

uint8_t op_pha(_nes* nes) {
    push(nes, nes->cpu.a);
	return 0;
}

uint8_t op_php(_nes* nes) {
    push(nes, nes->cpu.p | _B);
	return 0;
}

uint8_t op_pla(_nes* nes) {
    nes->cpu.a = pull(nes);
    set_flag(nes, _Z, nes->cpu.a == 0x00);
    set_flag(nes, _N, nes->cpu.a & 0x80);
	return 0;
}

uint8_t op_plp(_nes* nes) {
    nes->cpu.p = pull(nes) | _U;
    return 0;
}

uint8_t op_rol(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    uint8_t res = (memory << 1) | get_flag(nes, _C);

    set_flag(nes, _C, memory & 0x80);
    set_flag(nes, _Z, res == 0x00);
    set_flag(nes, _N, res & 0x80);

    cpu_write_back(nes, res);
	return 0;
}

uint8_t op_ror(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    uint8_t res = (memory >> 1) | (get_flag(nes, _C) << 7);

    set_flag(nes, _C, memory & 0x01);
    set_flag(nes, _Z, res == 0x00);
    set_flag(nes, _N, res & 0x80);

    cpu_write_back(nes, res);
	return 0;
}

uint8_t op_rti(_nes* nes) {
    nes->cpu.p = pull(nes) & ~_B & ~_U;
    nes->cpu.pc = pull(nes);
    nes->cpu.pc |= (uint16_t)pull(nes) << 8;
	return 0;
}

uint8_t op_rts(_nes* nes) {
    nes->cpu.pc = pull(nes);
    nes->cpu.pc |= (uint16_t)pull(nes) << 8;
    nes->cpu.pc++;
	return 0;
}

uint8_t op_sbc(_nes* nes) {
    uint8_t memory = cpu_fetch(nes);
    uint16_t value = (uint16_t)memory ^ 0xFF;
    uint16_t res = (uint16_t)nes->cpu.a + value + get_flag(nes, _C);

    uint16_t overflow = (res ^ nes->cpu.a) & (res ^ value) & 0x80;
    set_flag(nes, _C, res > 0xFF);
    set_flag(nes, _Z, (res & 0xFF) == 0x00);
    set_flag(nes, _V, overflow);
    set_flag(nes, _N, res & 0x80);

    nes->cpu.a = res & 0xFF;
    return 1;
}

uint8_t op_sec(_nes* nes) {
    set_flag(nes, _C, 1);
	return 0;
}

uint8_t op_sed(_nes* nes) {
    set_flag(nes, _D, 1);
	return 0;
}

uint8_t op_sei(_nes* nes) {
    set_flag(nes, _I, 1);
	return 0;
}

uint8_t op_sta(_nes* nes) {
    cpu_write(nes, nes->cpu.op_addr, nes->cpu.a);
	return 0;
}

uint8_t op_stx(_nes* nes) {
    cpu_write(nes, nes->cpu.op_addr, nes->cpu.x);
	return 0;
}

uint8_t op_sty(_nes* nes) {
    cpu_write(nes, nes->cpu.op_addr, nes->cpu.y);
	return 0;
}

uint8_t op_tax(_nes* nes) {
    nes->cpu.x = nes->cpu.a;
    set_flag(nes, _Z, nes->cpu.x == 0x00);
    set_flag(nes, _N, nes->cpu.x & 0x80);
	return 0;
}

uint8_t op_tay(_nes* nes) {
    nes->cpu.y = nes->cpu.a;
    set_flag(nes, _Z, nes->cpu.y == 0x00);
    set_flag(nes, _N, nes->cpu.y & 0x80);
	return 0;
}

uint8_t op_tsx(_nes* nes) {
    nes->cpu.x = nes->cpu.s;
    set_flag(nes, _Z, nes->cpu.x == 0x00);
    set_flag(nes, _N, nes->cpu.x & 0x80);
	return 0;
}

uint8_t op_txa(_nes* nes) {
    nes->cpu.a = nes->cpu.x;
    set_flag(nes, _Z, nes->cpu.a == 0x00);
    set_flag(nes, _N, nes->cpu.a & 0x80);
	return 0;
}

uint8_t op_txs(_nes* nes) {
    nes->cpu.s = nes->cpu.x;
	return 0;
}

uint8_t op_tya(_nes* nes) {
    nes->cpu.a = nes->cpu.y;
    set_flag(nes, _Z, nes->cpu.a == 0x00);
    set_flag(nes, _N, nes->cpu.a & 0x80);
	return 0;
}

uint8_t op____(_nes* nes) {
    fprintf(stderr, "ERROR: _Illegal instruction called!");
    return 0;
}
