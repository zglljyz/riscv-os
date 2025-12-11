
kernel.elf:     file format elf64-littleriscv


Disassembly of section .text:

0000000080000000 <_entry>:
    80000000:	00006117          	auipc	sp,0x6
    80000004:	30010113          	add	sp,sp,768 # 80006300 <freelist>
    80000008:	22200293          	li	t0,546
    8000000c:	30329073          	csrw	mideleg,t0
    80000010:	52fd                	li	t0,-1
    80000012:	30229073          	csrw	medeleg,t0
    80000016:	52fd                	li	t0,-1
    80000018:	3b029073          	csrw	pmpaddr0,t0
    8000001c:	42fd                	li	t0,31
    8000001e:	3a029073          	csrw	pmpcfg0,t0
    80000022:	300022f3          	csrr	t0,mstatus
    80000026:	6309                	lui	t1,0x2
    80000028:	8003031b          	addw	t1,t1,-2048 # 1800 <_entry-0x7fffe800>
    8000002c:	fff34313          	not	t1,t1
    80000030:	0062f2b3          	and	t0,t0,t1
    80000034:	6305                	lui	t1,0x1
    80000036:	8003031b          	addw	t1,t1,-2048 # 800 <_entry-0x7ffff800>
    8000003a:	0062e2b3          	or	t0,t0,t1
    8000003e:	30029073          	csrw	mstatus,t0
    80000042:	00000297          	auipc	t0,0x0
    80000046:	01028293          	add	t0,t0,16 # 80000052 <s_mode_entry>
    8000004a:	34129073          	csrw	mepc,t0
    8000004e:	30200073          	mret

0000000080000052 <s_mode_entry>:
    80000052:	00a000ef          	jal	8000005c <clear_bss>
    80000056:	024000ef          	jal	8000007a <main>

000000008000005a <spin>:
    8000005a:	a001                	j	8000005a <spin>

000000008000005c <clear_bss>:
    8000005c:	00006517          	auipc	a0,0x6
    80000060:	2a450513          	add	a0,a0,676 # 80006300 <freelist>
    80000064:	00008597          	auipc	a1,0x8
    80000068:	2b458593          	add	a1,a1,692 # 80008318 <_bss_end>

000000008000006c <bss_clear_loop>:
    8000006c:	00b50663          	beq	a0,a1,80000078 <bss_clear_done>
    80000070:	00053023          	sd	zero,0(a0)
    80000074:	0521                	add	a0,a0,8
    80000076:	bfdd                	j	8000006c <bss_clear_loop>

0000000080000078 <bss_clear_done>:
    80000078:	8082                	ret

000000008000007a <main>:
    8000007a:	1141                	add	sp,sp,-16
    8000007c:	e406                	sd	ra,8(sp)
    8000007e:	e022                	sd	s0,0(sp)
    80000080:	0800                	add	s0,sp,16
    80000082:	00001517          	auipc	a0,0x1
    80000086:	74e50513          	add	a0,a0,1870 # 800017d0 <etext+0x4>
    8000008a:	3ba000ef          	jal	80000444 <printf>
    8000008e:	526000ef          	jal	800005b4 <pmm_init>
    80000092:	00001517          	auipc	a0,0x1
    80000096:	75e50513          	add	a0,a0,1886 # 800017f0 <etext+0x24>
    8000009a:	3aa000ef          	jal	80000444 <printf>
    8000009e:	3c5000ef          	jal	80000c62 <kvminit>
    800000a2:	00001517          	auipc	a0,0x1
    800000a6:	76650513          	add	a0,a0,1894 # 80001808 <etext+0x3c>
    800000aa:	39a000ef          	jal	80000444 <printf>
    800000ae:	477000ef          	jal	80000d24 <kvminithart>
    800000b2:	00001517          	auipc	a0,0x1
    800000b6:	77650513          	add	a0,a0,1910 # 80001828 <etext+0x5c>
    800000ba:	38a000ef          	jal	80000444 <printf>
    800000be:	51e010ef          	jal	800015dc <trap_init_s>
    800000c2:	00001517          	auipc	a0,0x1
    800000c6:	77e50513          	add	a0,a0,1918 # 80001840 <etext+0x74>
    800000ca:	37a000ef          	jal	80000444 <printf>
    800000ce:	0001                	nop
    800000d0:	bffd                	j	800000ce <main+0x54>

00000000800000d2 <uart_putc>:
    800000d2:	1101                	add	sp,sp,-32
    800000d4:	ec22                	sd	s0,24(sp)
    800000d6:	1000                	add	s0,sp,32
    800000d8:	87aa                	mv	a5,a0
    800000da:	fef407a3          	sb	a5,-17(s0)
    800000de:	0001                	nop
    800000e0:	100007b7          	lui	a5,0x10000
    800000e4:	0795                	add	a5,a5,5 # 10000005 <_entry-0x6ffffffb>
    800000e6:	0007c783          	lbu	a5,0(a5)
    800000ea:	0ff7f793          	zext.b	a5,a5
    800000ee:	2781                	sext.w	a5,a5
    800000f0:	0207f793          	and	a5,a5,32
    800000f4:	2781                	sext.w	a5,a5
    800000f6:	d7ed                	beqz	a5,800000e0 <uart_putc+0xe>
    800000f8:	100007b7          	lui	a5,0x10000
    800000fc:	fef44703          	lbu	a4,-17(s0)
    80000100:	00e78023          	sb	a4,0(a5) # 10000000 <_entry-0x70000000>
    80000104:	0001                	nop
    80000106:	6462                	ld	s0,24(sp)
    80000108:	6105                	add	sp,sp,32
    8000010a:	8082                	ret

000000008000010c <uart_puts>:
    8000010c:	1101                	add	sp,sp,-32
    8000010e:	ec06                	sd	ra,24(sp)
    80000110:	e822                	sd	s0,16(sp)
    80000112:	1000                	add	s0,sp,32
    80000114:	fea43423          	sd	a0,-24(s0)
    80000118:	a829                	j	80000132 <uart_puts+0x26>
    8000011a:	fe843783          	ld	a5,-24(s0)
    8000011e:	0007c783          	lbu	a5,0(a5)
    80000122:	853e                	mv	a0,a5
    80000124:	fafff0ef          	jal	800000d2 <uart_putc>
    80000128:	fe843783          	ld	a5,-24(s0)
    8000012c:	0785                	add	a5,a5,1
    8000012e:	fef43423          	sd	a5,-24(s0)
    80000132:	fe843783          	ld	a5,-24(s0)
    80000136:	0007c783          	lbu	a5,0(a5)
    8000013a:	f3e5                	bnez	a5,8000011a <uart_puts+0xe>
    8000013c:	0001                	nop
    8000013e:	0001                	nop
    80000140:	60e2                	ld	ra,24(sp)
    80000142:	6442                	ld	s0,16(sp)
    80000144:	6105                	add	sp,sp,32
    80000146:	8082                	ret

0000000080000148 <print_number>:
    80000148:	715d                	add	sp,sp,-80
    8000014a:	e486                	sd	ra,72(sp)
    8000014c:	e0a2                	sd	s0,64(sp)
    8000014e:	0880                	add	s0,sp,80
    80000150:	faa43c23          	sd	a0,-72(s0)
    80000154:	87ae                	mv	a5,a1
    80000156:	8732                	mv	a4,a2
    80000158:	faf42a23          	sw	a5,-76(s0)
    8000015c:	87ba                	mv	a5,a4
    8000015e:	faf42823          	sw	a5,-80(s0)
    80000162:	fe042623          	sw	zero,-20(s0)
    80000166:	fc042e23          	sw	zero,-36(s0)
    8000016a:	fb042783          	lw	a5,-80(s0)
    8000016e:	2781                	sext.w	a5,a5
    80000170:	cf99                	beqz	a5,8000018e <print_number+0x46>
    80000172:	fb843783          	ld	a5,-72(s0)
    80000176:	0007dc63          	bgez	a5,8000018e <print_number+0x46>
    8000017a:	4785                	li	a5,1
    8000017c:	fcf42e23          	sw	a5,-36(s0)
    80000180:	fb843783          	ld	a5,-72(s0)
    80000184:	40f007b3          	neg	a5,a5
    80000188:	fef43023          	sd	a5,-32(s0)
    8000018c:	a029                	j	80000196 <print_number+0x4e>
    8000018e:	fb843783          	ld	a5,-72(s0)
    80000192:	fef43023          	sd	a5,-32(s0)
    80000196:	fb442783          	lw	a5,-76(s0)
    8000019a:	fe043703          	ld	a4,-32(s0)
    8000019e:	02f77733          	remu	a4,a4,a5
    800001a2:	fec42783          	lw	a5,-20(s0)
    800001a6:	0017869b          	addw	a3,a5,1
    800001aa:	fed42623          	sw	a3,-20(s0)
    800001ae:	00002697          	auipc	a3,0x2
    800001b2:	13268693          	add	a3,a3,306 # 800022e0 <digits>
    800001b6:	9736                	add	a4,a4,a3
    800001b8:	00074703          	lbu	a4,0(a4)
    800001bc:	17c1                	add	a5,a5,-16
    800001be:	97a2                	add	a5,a5,s0
    800001c0:	fce78c23          	sb	a4,-40(a5)
    800001c4:	fb442783          	lw	a5,-76(s0)
    800001c8:	fe043703          	ld	a4,-32(s0)
    800001cc:	02f757b3          	divu	a5,a4,a5
    800001d0:	fef43023          	sd	a5,-32(s0)
    800001d4:	fe043783          	ld	a5,-32(s0)
    800001d8:	ffdd                	bnez	a5,80000196 <print_number+0x4e>
    800001da:	fdc42783          	lw	a5,-36(s0)
    800001de:	2781                	sext.w	a5,a5
    800001e0:	c79d                	beqz	a5,8000020e <print_number+0xc6>
    800001e2:	fec42783          	lw	a5,-20(s0)
    800001e6:	0017871b          	addw	a4,a5,1
    800001ea:	fee42623          	sw	a4,-20(s0)
    800001ee:	17c1                	add	a5,a5,-16
    800001f0:	97a2                	add	a5,a5,s0
    800001f2:	02d00713          	li	a4,45
    800001f6:	fce78c23          	sb	a4,-40(a5)
    800001fa:	a811                	j	8000020e <print_number+0xc6>
    800001fc:	fec42783          	lw	a5,-20(s0)
    80000200:	17c1                	add	a5,a5,-16
    80000202:	97a2                	add	a5,a5,s0
    80000204:	fd87c783          	lbu	a5,-40(a5)
    80000208:	853e                	mv	a0,a5
    8000020a:	ec9ff0ef          	jal	800000d2 <uart_putc>
    8000020e:	fec42783          	lw	a5,-20(s0)
    80000212:	37fd                	addw	a5,a5,-1
    80000214:	fef42623          	sw	a5,-20(s0)
    80000218:	fec42783          	lw	a5,-20(s0)
    8000021c:	2781                	sext.w	a5,a5
    8000021e:	fc07dfe3          	bgez	a5,800001fc <print_number+0xb4>
    80000222:	0001                	nop
    80000224:	0001                	nop
    80000226:	60a6                	ld	ra,72(sp)
    80000228:	6406                	ld	s0,64(sp)
    8000022a:	6161                	add	sp,sp,80
    8000022c:	8082                	ret

000000008000022e <vprintf>:
    8000022e:	711d                	add	sp,sp,-96
    80000230:	ec86                	sd	ra,88(sp)
    80000232:	e8a2                	sd	s0,80(sp)
    80000234:	1080                	add	s0,sp,96
    80000236:	faa43423          	sd	a0,-88(s0)
    8000023a:	fab43023          	sd	a1,-96(s0)
    8000023e:	fa843783          	ld	a5,-88(s0)
    80000242:	fef43423          	sd	a5,-24(s0)
    80000246:	fe042223          	sw	zero,-28(s0)
    8000024a:	a2cd                	j	8000042c <vprintf+0x1fe>
    8000024c:	fe843783          	ld	a5,-24(s0)
    80000250:	0007c783          	lbu	a5,0(a5)
    80000254:	873e                	mv	a4,a5
    80000256:	02500793          	li	a5,37
    8000025a:	00f70f63          	beq	a4,a5,80000278 <vprintf+0x4a>
    8000025e:	fe843783          	ld	a5,-24(s0)
    80000262:	0007c783          	lbu	a5,0(a5)
    80000266:	853e                	mv	a0,a5
    80000268:	e6bff0ef          	jal	800000d2 <uart_putc>
    8000026c:	fe843783          	ld	a5,-24(s0)
    80000270:	0785                	add	a5,a5,1
    80000272:	fef43423          	sd	a5,-24(s0)
    80000276:	aa5d                	j	8000042c <vprintf+0x1fe>
    80000278:	fe843783          	ld	a5,-24(s0)
    8000027c:	0785                	add	a5,a5,1
    8000027e:	fef43423          	sd	a5,-24(s0)
    80000282:	fe843783          	ld	a5,-24(s0)
    80000286:	0007c783          	lbu	a5,0(a5)
    8000028a:	873e                	mv	a4,a5
    8000028c:	06c00793          	li	a5,108
    80000290:	00f71a63          	bne	a4,a5,800002a4 <vprintf+0x76>
    80000294:	4785                	li	a5,1
    80000296:	fef42223          	sw	a5,-28(s0)
    8000029a:	fe843783          	ld	a5,-24(s0)
    8000029e:	0785                	add	a5,a5,1
    800002a0:	fef43423          	sd	a5,-24(s0)
    800002a4:	fe843783          	ld	a5,-24(s0)
    800002a8:	0007c783          	lbu	a5,0(a5)
    800002ac:	2781                	sext.w	a5,a5
    800002ae:	86be                	mv	a3,a5
    800002b0:	02500713          	li	a4,37
    800002b4:	14e68463          	beq	a3,a4,800003fc <vprintf+0x1ce>
    800002b8:	86be                	mv	a3,a5
    800002ba:	02500713          	li	a4,37
    800002be:	14e6c463          	blt	a3,a4,80000406 <vprintf+0x1d8>
    800002c2:	86be                	mv	a3,a5
    800002c4:	07800713          	li	a4,120
    800002c8:	12d74f63          	blt	a4,a3,80000406 <vprintf+0x1d8>
    800002cc:	86be                	mv	a3,a5
    800002ce:	06300713          	li	a4,99
    800002d2:	12e6ca63          	blt	a3,a4,80000406 <vprintf+0x1d8>
    800002d6:	f9d7869b          	addw	a3,a5,-99
    800002da:	0006871b          	sext.w	a4,a3
    800002de:	47d5                	li	a5,21
    800002e0:	12e7e363          	bltu	a5,a4,80000406 <vprintf+0x1d8>
    800002e4:	02069793          	sll	a5,a3,0x20
    800002e8:	9381                	srl	a5,a5,0x20
    800002ea:	00279713          	sll	a4,a5,0x2
    800002ee:	00001797          	auipc	a5,0x1
    800002f2:	59278793          	add	a5,a5,1426 # 80001880 <etext+0xb4>
    800002f6:	97ba                	add	a5,a5,a4
    800002f8:	439c                	lw	a5,0(a5)
    800002fa:	0007871b          	sext.w	a4,a5
    800002fe:	00001797          	auipc	a5,0x1
    80000302:	58278793          	add	a5,a5,1410 # 80001880 <etext+0xb4>
    80000306:	97ba                	add	a5,a5,a4
    80000308:	8782                	jr	a5
    8000030a:	fe442783          	lw	a5,-28(s0)
    8000030e:	2781                	sext.w	a5,a5
    80000310:	cb89                	beqz	a5,80000322 <vprintf+0xf4>
    80000312:	fa043783          	ld	a5,-96(s0)
    80000316:	00878713          	add	a4,a5,8
    8000031a:	fae43023          	sd	a4,-96(s0)
    8000031e:	639c                	ld	a5,0(a5)
    80000320:	a801                	j	80000330 <vprintf+0x102>
    80000322:	fa043783          	ld	a5,-96(s0)
    80000326:	00878713          	add	a4,a5,8
    8000032a:	fae43023          	sd	a4,-96(s0)
    8000032e:	439c                	lw	a5,0(a5)
    80000330:	fcf43023          	sd	a5,-64(s0)
    80000334:	4605                	li	a2,1
    80000336:	45a9                	li	a1,10
    80000338:	fc043503          	ld	a0,-64(s0)
    8000033c:	e0dff0ef          	jal	80000148 <print_number>
    80000340:	a8f9                	j	8000041e <vprintf+0x1f0>
    80000342:	fe442783          	lw	a5,-28(s0)
    80000346:	2781                	sext.w	a5,a5
    80000348:	cb89                	beqz	a5,8000035a <vprintf+0x12c>
    8000034a:	fa043783          	ld	a5,-96(s0)
    8000034e:	00878713          	add	a4,a5,8
    80000352:	fae43023          	sd	a4,-96(s0)
    80000356:	639c                	ld	a5,0(a5)
    80000358:	a811                	j	8000036c <vprintf+0x13e>
    8000035a:	fa043783          	ld	a5,-96(s0)
    8000035e:	00878713          	add	a4,a5,8
    80000362:	fae43023          	sd	a4,-96(s0)
    80000366:	439c                	lw	a5,0(a5)
    80000368:	1782                	sll	a5,a5,0x20
    8000036a:	9381                	srl	a5,a5,0x20
    8000036c:	fcf43c23          	sd	a5,-40(s0)
    80000370:	fd843783          	ld	a5,-40(s0)
    80000374:	4601                	li	a2,0
    80000376:	45c1                	li	a1,16
    80000378:	853e                	mv	a0,a5
    8000037a:	dcfff0ef          	jal	80000148 <print_number>
    8000037e:	a045                	j	8000041e <vprintf+0x1f0>
    80000380:	fa043783          	ld	a5,-96(s0)
    80000384:	00878713          	add	a4,a5,8
    80000388:	fae43023          	sd	a4,-96(s0)
    8000038c:	639c                	ld	a5,0(a5)
    8000038e:	fcf43423          	sd	a5,-56(s0)
    80000392:	00001517          	auipc	a0,0x1
    80000396:	4de50513          	add	a0,a0,1246 # 80001870 <etext+0xa4>
    8000039a:	d73ff0ef          	jal	8000010c <uart_puts>
    8000039e:	fc843783          	ld	a5,-56(s0)
    800003a2:	4601                	li	a2,0
    800003a4:	45c1                	li	a1,16
    800003a6:	853e                	mv	a0,a5
    800003a8:	da1ff0ef          	jal	80000148 <print_number>
    800003ac:	a88d                	j	8000041e <vprintf+0x1f0>
    800003ae:	fa043783          	ld	a5,-96(s0)
    800003b2:	00878713          	add	a4,a5,8
    800003b6:	fae43023          	sd	a4,-96(s0)
    800003ba:	639c                	ld	a5,0(a5)
    800003bc:	fcf43823          	sd	a5,-48(s0)
    800003c0:	fd043783          	ld	a5,-48(s0)
    800003c4:	eb81                	bnez	a5,800003d4 <vprintf+0x1a6>
    800003c6:	00001517          	auipc	a0,0x1
    800003ca:	4b250513          	add	a0,a0,1202 # 80001878 <etext+0xac>
    800003ce:	d3fff0ef          	jal	8000010c <uart_puts>
    800003d2:	a0b1                	j	8000041e <vprintf+0x1f0>
    800003d4:	fd043503          	ld	a0,-48(s0)
    800003d8:	d35ff0ef          	jal	8000010c <uart_puts>
    800003dc:	a089                	j	8000041e <vprintf+0x1f0>
    800003de:	fa043783          	ld	a5,-96(s0)
    800003e2:	00878713          	add	a4,a5,8
    800003e6:	fae43023          	sd	a4,-96(s0)
    800003ea:	439c                	lw	a5,0(a5)
    800003ec:	faf40fa3          	sb	a5,-65(s0)
    800003f0:	fbf44783          	lbu	a5,-65(s0)
    800003f4:	853e                	mv	a0,a5
    800003f6:	cddff0ef          	jal	800000d2 <uart_putc>
    800003fa:	a015                	j	8000041e <vprintf+0x1f0>
    800003fc:	02500513          	li	a0,37
    80000400:	cd3ff0ef          	jal	800000d2 <uart_putc>
    80000404:	a829                	j	8000041e <vprintf+0x1f0>
    80000406:	02500513          	li	a0,37
    8000040a:	cc9ff0ef          	jal	800000d2 <uart_putc>
    8000040e:	fe843783          	ld	a5,-24(s0)
    80000412:	0007c783          	lbu	a5,0(a5)
    80000416:	853e                	mv	a0,a5
    80000418:	cbbff0ef          	jal	800000d2 <uart_putc>
    8000041c:	0001                	nop
    8000041e:	fe042223          	sw	zero,-28(s0)
    80000422:	fe843783          	ld	a5,-24(s0)
    80000426:	0785                	add	a5,a5,1
    80000428:	fef43423          	sd	a5,-24(s0)
    8000042c:	fe843783          	ld	a5,-24(s0)
    80000430:	0007c783          	lbu	a5,0(a5)
    80000434:	e0079ce3          	bnez	a5,8000024c <vprintf+0x1e>
    80000438:	0001                	nop
    8000043a:	0001                	nop
    8000043c:	60e6                	ld	ra,88(sp)
    8000043e:	6446                	ld	s0,80(sp)
    80000440:	6125                	add	sp,sp,96
    80000442:	8082                	ret

0000000080000444 <printf>:
    80000444:	7159                	add	sp,sp,-112
    80000446:	f406                	sd	ra,40(sp)
    80000448:	f022                	sd	s0,32(sp)
    8000044a:	1800                	add	s0,sp,48
    8000044c:	fca43c23          	sd	a0,-40(s0)
    80000450:	e40c                	sd	a1,8(s0)
    80000452:	e810                	sd	a2,16(s0)
    80000454:	ec14                	sd	a3,24(s0)
    80000456:	f018                	sd	a4,32(s0)
    80000458:	f41c                	sd	a5,40(s0)
    8000045a:	03043823          	sd	a6,48(s0)
    8000045e:	03143c23          	sd	a7,56(s0)
    80000462:	04040793          	add	a5,s0,64
    80000466:	fcf43823          	sd	a5,-48(s0)
    8000046a:	fd043783          	ld	a5,-48(s0)
    8000046e:	fc878793          	add	a5,a5,-56
    80000472:	fef43423          	sd	a5,-24(s0)
    80000476:	fe843783          	ld	a5,-24(s0)
    8000047a:	85be                	mv	a1,a5
    8000047c:	fd843503          	ld	a0,-40(s0)
    80000480:	dafff0ef          	jal	8000022e <vprintf>
    80000484:	0001                	nop
    80000486:	70a2                	ld	ra,40(sp)
    80000488:	7402                	ld	s0,32(sp)
    8000048a:	6165                	add	sp,sp,112
    8000048c:	8082                	ret

000000008000048e <clear_screen>:
    8000048e:	1141                	add	sp,sp,-16
    80000490:	e406                	sd	ra,8(sp)
    80000492:	e022                	sd	s0,0(sp)
    80000494:	0800                	add	s0,sp,16
    80000496:	00001517          	auipc	a0,0x1
    8000049a:	44250513          	add	a0,a0,1090 # 800018d8 <etext+0x10c>
    8000049e:	fa7ff0ef          	jal	80000444 <printf>
    800004a2:	0001                	nop
    800004a4:	60a2                	ld	ra,8(sp)
    800004a6:	6402                	ld	s0,0(sp)
    800004a8:	0141                	add	sp,sp,16
    800004aa:	8082                	ret

00000000800004ac <goto_xy>:
    800004ac:	1101                	add	sp,sp,-32
    800004ae:	ec06                	sd	ra,24(sp)
    800004b0:	e822                	sd	s0,16(sp)
    800004b2:	1000                	add	s0,sp,32
    800004b4:	87aa                	mv	a5,a0
    800004b6:	872e                	mv	a4,a1
    800004b8:	fef42623          	sw	a5,-20(s0)
    800004bc:	87ba                	mv	a5,a4
    800004be:	fef42423          	sw	a5,-24(s0)
    800004c2:	fec42703          	lw	a4,-20(s0)
    800004c6:	fe842783          	lw	a5,-24(s0)
    800004ca:	863a                	mv	a2,a4
    800004cc:	85be                	mv	a1,a5
    800004ce:	00001517          	auipc	a0,0x1
    800004d2:	41250513          	add	a0,a0,1042 # 800018e0 <etext+0x114>
    800004d6:	f6fff0ef          	jal	80000444 <printf>
    800004da:	0001                	nop
    800004dc:	60e2                	ld	ra,24(sp)
    800004de:	6442                	ld	s0,16(sp)
    800004e0:	6105                	add	sp,sp,32
    800004e2:	8082                	ret

00000000800004e4 <clear_line>:
    800004e4:	1141                	add	sp,sp,-16
    800004e6:	e406                	sd	ra,8(sp)
    800004e8:	e022                	sd	s0,0(sp)
    800004ea:	0800                	add	s0,sp,16
    800004ec:	00001517          	auipc	a0,0x1
    800004f0:	40450513          	add	a0,a0,1028 # 800018f0 <etext+0x124>
    800004f4:	f51ff0ef          	jal	80000444 <printf>
    800004f8:	0001                	nop
    800004fa:	60a2                	ld	ra,8(sp)
    800004fc:	6402                	ld	s0,0(sp)
    800004fe:	0141                	add	sp,sp,16
    80000500:	8082                	ret

0000000080000502 <memset>:
    80000502:	7139                	add	sp,sp,-64
    80000504:	fc22                	sd	s0,56(sp)
    80000506:	0080                	add	s0,sp,64
    80000508:	fca43c23          	sd	a0,-40(s0)
    8000050c:	87ae                	mv	a5,a1
    8000050e:	fcc43423          	sd	a2,-56(s0)
    80000512:	fcf42a23          	sw	a5,-44(s0)
    80000516:	fd843783          	ld	a5,-40(s0)
    8000051a:	fef43423          	sd	a5,-24(s0)
    8000051e:	a015                	j	80000542 <memset+0x40>
    80000520:	fe843783          	ld	a5,-24(s0)
    80000524:	00178713          	add	a4,a5,1
    80000528:	fee43423          	sd	a4,-24(s0)
    8000052c:	fd442703          	lw	a4,-44(s0)
    80000530:	0ff77713          	zext.b	a4,a4
    80000534:	00e78023          	sb	a4,0(a5)
    80000538:	fc843783          	ld	a5,-56(s0)
    8000053c:	17fd                	add	a5,a5,-1
    8000053e:	fcf43423          	sd	a5,-56(s0)
    80000542:	fc843783          	ld	a5,-56(s0)
    80000546:	ffe9                	bnez	a5,80000520 <memset+0x1e>
    80000548:	fd843783          	ld	a5,-40(s0)
    8000054c:	853e                	mv	a0,a5
    8000054e:	7462                	ld	s0,56(sp)
    80000550:	6121                	add	sp,sp,64
    80000552:	8082                	ret

0000000080000554 <memcpy>:
    80000554:	7139                	add	sp,sp,-64
    80000556:	fc22                	sd	s0,56(sp)
    80000558:	0080                	add	s0,sp,64
    8000055a:	fca43c23          	sd	a0,-40(s0)
    8000055e:	fcb43823          	sd	a1,-48(s0)
    80000562:	fcc43423          	sd	a2,-56(s0)
    80000566:	fd843783          	ld	a5,-40(s0)
    8000056a:	fef43423          	sd	a5,-24(s0)
    8000056e:	fd043783          	ld	a5,-48(s0)
    80000572:	fef43023          	sd	a5,-32(s0)
    80000576:	a035                	j	800005a2 <memcpy+0x4e>
    80000578:	fe043703          	ld	a4,-32(s0)
    8000057c:	00170793          	add	a5,a4,1
    80000580:	fef43023          	sd	a5,-32(s0)
    80000584:	fe843783          	ld	a5,-24(s0)
    80000588:	00178693          	add	a3,a5,1
    8000058c:	fed43423          	sd	a3,-24(s0)
    80000590:	00074703          	lbu	a4,0(a4)
    80000594:	00e78023          	sb	a4,0(a5)
    80000598:	fc843783          	ld	a5,-56(s0)
    8000059c:	17fd                	add	a5,a5,-1
    8000059e:	fcf43423          	sd	a5,-56(s0)
    800005a2:	fc843783          	ld	a5,-56(s0)
    800005a6:	fbe9                	bnez	a5,80000578 <memcpy+0x24>
    800005a8:	fd843783          	ld	a5,-40(s0)
    800005ac:	853e                	mv	a0,a5
    800005ae:	7462                	ld	s0,56(sp)
    800005b0:	6121                	add	sp,sp,64
    800005b2:	8082                	ret

00000000800005b4 <pmm_init>:
    800005b4:	1101                	add	sp,sp,-32
    800005b6:	ec06                	sd	ra,24(sp)
    800005b8:	e822                	sd	s0,16(sp)
    800005ba:	1000                	add	s0,sp,32
    800005bc:	00006797          	auipc	a5,0x6
    800005c0:	d4478793          	add	a5,a5,-700 # 80006300 <freelist>
    800005c4:	0007b023          	sd	zero,0(a5)
    800005c8:	00008717          	auipc	a4,0x8
    800005cc:	d5070713          	add	a4,a4,-688 # 80008318 <_bss_end>
    800005d0:	6785                	lui	a5,0x1
    800005d2:	17fd                	add	a5,a5,-1 # fff <_entry-0x7ffff001>
    800005d4:	973e                	add	a4,a4,a5
    800005d6:	77fd                	lui	a5,0xfffff
    800005d8:	8ff9                	and	a5,a5,a4
    800005da:	fef43423          	sd	a5,-24(s0)
    800005de:	a819                	j	800005f4 <pmm_init+0x40>
    800005e0:	fe843503          	ld	a0,-24(s0)
    800005e4:	030000ef          	jal	80000614 <free_page>
    800005e8:	fe843703          	ld	a4,-24(s0)
    800005ec:	6785                	lui	a5,0x1
    800005ee:	97ba                	add	a5,a5,a4
    800005f0:	fef43423          	sd	a5,-24(s0)
    800005f4:	fe843703          	ld	a4,-24(s0)
    800005f8:	6785                	lui	a5,0x1
    800005fa:	973e                	add	a4,a4,a5
    800005fc:	08000797          	auipc	a5,0x8000
    80000600:	a0478793          	add	a5,a5,-1532 # 88000000 <PHYSTOP>
    80000604:	fce7fee3          	bgeu	a5,a4,800005e0 <pmm_init+0x2c>
    80000608:	0001                	nop
    8000060a:	0001                	nop
    8000060c:	60e2                	ld	ra,24(sp)
    8000060e:	6442                	ld	s0,16(sp)
    80000610:	6105                	add	sp,sp,32
    80000612:	8082                	ret

0000000080000614 <free_page>:
    80000614:	7179                	add	sp,sp,-48
    80000616:	f406                	sd	ra,40(sp)
    80000618:	f022                	sd	s0,32(sp)
    8000061a:	1800                	add	s0,sp,48
    8000061c:	fca43c23          	sd	a0,-40(s0)
    80000620:	fd843703          	ld	a4,-40(s0)
    80000624:	6785                	lui	a5,0x1
    80000626:	17fd                	add	a5,a5,-1 # fff <_entry-0x7ffff001>
    80000628:	8ff9                	and	a5,a5,a4
    8000062a:	e38d                	bnez	a5,8000064c <free_page+0x38>
    8000062c:	fd843703          	ld	a4,-40(s0)
    80000630:	00008797          	auipc	a5,0x8
    80000634:	ce878793          	add	a5,a5,-792 # 80008318 <_bss_end>
    80000638:	00f76a63          	bltu	a4,a5,8000064c <free_page+0x38>
    8000063c:	fd843703          	ld	a4,-40(s0)
    80000640:	08000797          	auipc	a5,0x8000
    80000644:	9c078793          	add	a5,a5,-1600 # 88000000 <PHYSTOP>
    80000648:	00f76a63          	bltu	a4,a5,8000065c <free_page+0x48>
    8000064c:	00001517          	auipc	a0,0x1
    80000650:	2ac50513          	add	a0,a0,684 # 800018f8 <etext+0x12c>
    80000654:	df1ff0ef          	jal	80000444 <printf>
    80000658:	0001                	nop
    8000065a:	bffd                	j	80000658 <free_page+0x44>
    8000065c:	6605                	lui	a2,0x1
    8000065e:	4585                	li	a1,1
    80000660:	fd843503          	ld	a0,-40(s0)
    80000664:	e9fff0ef          	jal	80000502 <memset>
    80000668:	fd843783          	ld	a5,-40(s0)
    8000066c:	fef43423          	sd	a5,-24(s0)
    80000670:	00006797          	auipc	a5,0x6
    80000674:	c9078793          	add	a5,a5,-880 # 80006300 <freelist>
    80000678:	6398                	ld	a4,0(a5)
    8000067a:	fe843783          	ld	a5,-24(s0)
    8000067e:	e398                	sd	a4,0(a5)
    80000680:	00006797          	auipc	a5,0x6
    80000684:	c8078793          	add	a5,a5,-896 # 80006300 <freelist>
    80000688:	fe843703          	ld	a4,-24(s0)
    8000068c:	e398                	sd	a4,0(a5)
    8000068e:	0001                	nop
    80000690:	70a2                	ld	ra,40(sp)
    80000692:	7402                	ld	s0,32(sp)
    80000694:	6145                	add	sp,sp,48
    80000696:	8082                	ret

0000000080000698 <alloc_page>:
    80000698:	1101                	add	sp,sp,-32
    8000069a:	ec06                	sd	ra,24(sp)
    8000069c:	e822                	sd	s0,16(sp)
    8000069e:	1000                	add	s0,sp,32
    800006a0:	00006797          	auipc	a5,0x6
    800006a4:	c6078793          	add	a5,a5,-928 # 80006300 <freelist>
    800006a8:	639c                	ld	a5,0(a5)
    800006aa:	fef43423          	sd	a5,-24(s0)
    800006ae:	fe843783          	ld	a5,-24(s0)
    800006b2:	cb89                	beqz	a5,800006c4 <alloc_page+0x2c>
    800006b4:	fe843783          	ld	a5,-24(s0)
    800006b8:	6398                	ld	a4,0(a5)
    800006ba:	00006797          	auipc	a5,0x6
    800006be:	c4678793          	add	a5,a5,-954 # 80006300 <freelist>
    800006c2:	e398                	sd	a4,0(a5)
    800006c4:	fe843783          	ld	a5,-24(s0)
    800006c8:	c799                	beqz	a5,800006d6 <alloc_page+0x3e>
    800006ca:	6605                	lui	a2,0x1
    800006cc:	4595                	li	a1,5
    800006ce:	fe843503          	ld	a0,-24(s0)
    800006d2:	e31ff0ef          	jal	80000502 <memset>
    800006d6:	fe843783          	ld	a5,-24(s0)
    800006da:	853e                	mv	a0,a5
    800006dc:	60e2                	ld	ra,24(sp)
    800006de:	6442                	ld	s0,16(sp)
    800006e0:	6105                	add	sp,sp,32
    800006e2:	8082                	ret

00000000800006e4 <test_pmm>:
    800006e4:	7179                	add	sp,sp,-48
    800006e6:	f406                	sd	ra,40(sp)
    800006e8:	f022                	sd	s0,32(sp)
    800006ea:	1800                	add	s0,sp,48
    800006ec:	00001517          	auipc	a0,0x1
    800006f0:	22c50513          	add	a0,a0,556 # 80001918 <etext+0x14c>
    800006f4:	d51ff0ef          	jal	80000444 <printf>
    800006f8:	fa1ff0ef          	jal	80000698 <alloc_page>
    800006fc:	fea43423          	sd	a0,-24(s0)
    80000700:	fe843583          	ld	a1,-24(s0)
    80000704:	00001517          	auipc	a0,0x1
    80000708:	23c50513          	add	a0,a0,572 # 80001940 <etext+0x174>
    8000070c:	d39ff0ef          	jal	80000444 <printf>
    80000710:	f89ff0ef          	jal	80000698 <alloc_page>
    80000714:	fea43023          	sd	a0,-32(s0)
    80000718:	fe043583          	ld	a1,-32(s0)
    8000071c:	00001517          	auipc	a0,0x1
    80000720:	24450513          	add	a0,a0,580 # 80001960 <etext+0x194>
    80000724:	d21ff0ef          	jal	80000444 <printf>
    80000728:	fe843783          	ld	a5,-24(s0)
    8000072c:	c781                	beqz	a5,80000734 <test_pmm+0x50>
    8000072e:	fe043783          	ld	a5,-32(s0)
    80000732:	eb81                	bnez	a5,80000742 <test_pmm+0x5e>
    80000734:	00001517          	auipc	a0,0x1
    80000738:	24c50513          	add	a0,a0,588 # 80001980 <etext+0x1b4>
    8000073c:	d09ff0ef          	jal	80000444 <printf>
    80000740:	a069                	j	800007ca <test_pmm+0xe6>
    80000742:	fe843703          	ld	a4,-24(s0)
    80000746:	fe043783          	ld	a5,-32(s0)
    8000074a:	00f71963          	bne	a4,a5,8000075c <test_pmm+0x78>
    8000074e:	00001517          	auipc	a0,0x1
    80000752:	25a50513          	add	a0,a0,602 # 800019a8 <etext+0x1dc>
    80000756:	cefff0ef          	jal	80000444 <printf>
    8000075a:	a885                	j	800007ca <test_pmm+0xe6>
    8000075c:	fe843503          	ld	a0,-24(s0)
    80000760:	eb5ff0ef          	jal	80000614 <free_page>
    80000764:	00001517          	auipc	a0,0x1
    80000768:	27c50513          	add	a0,a0,636 # 800019e0 <etext+0x214>
    8000076c:	cd9ff0ef          	jal	80000444 <printf>
    80000770:	f29ff0ef          	jal	80000698 <alloc_page>
    80000774:	fca43c23          	sd	a0,-40(s0)
    80000778:	fd843583          	ld	a1,-40(s0)
    8000077c:	00001517          	auipc	a0,0x1
    80000780:	27c50513          	add	a0,a0,636 # 800019f8 <etext+0x22c>
    80000784:	cc1ff0ef          	jal	80000444 <printf>
    80000788:	fe843703          	ld	a4,-24(s0)
    8000078c:	fd843783          	ld	a5,-40(s0)
    80000790:	00f71963          	bne	a4,a5,800007a2 <test_pmm+0xbe>
    80000794:	00001517          	auipc	a0,0x1
    80000798:	28450513          	add	a0,a0,644 # 80001a18 <etext+0x24c>
    8000079c:	ca9ff0ef          	jal	80000444 <printf>
    800007a0:	a039                	j	800007ae <test_pmm+0xca>
    800007a2:	00001517          	auipc	a0,0x1
    800007a6:	2ae50513          	add	a0,a0,686 # 80001a50 <etext+0x284>
    800007aa:	c9bff0ef          	jal	80000444 <printf>
    800007ae:	fe043503          	ld	a0,-32(s0)
    800007b2:	e63ff0ef          	jal	80000614 <free_page>
    800007b6:	fd843503          	ld	a0,-40(s0)
    800007ba:	e5bff0ef          	jal	80000614 <free_page>
    800007be:	00001517          	auipc	a0,0x1
    800007c2:	2da50513          	add	a0,a0,730 # 80001a98 <etext+0x2cc>
    800007c6:	c7fff0ef          	jal	80000444 <printf>
    800007ca:	70a2                	ld	ra,40(sp)
    800007cc:	7402                	ld	s0,32(sp)
    800007ce:	6145                	add	sp,sp,48
    800007d0:	8082                	ret

00000000800007d2 <test_pmm_consistency>:
    800007d2:	7179                	add	sp,sp,-48
    800007d4:	f406                	sd	ra,40(sp)
    800007d6:	f022                	sd	s0,32(sp)
    800007d8:	1800                	add	s0,sp,48
    800007da:	00001517          	auipc	a0,0x1
    800007de:	2e650513          	add	a0,a0,742 # 80001ac0 <etext+0x2f4>
    800007e2:	c63ff0ef          	jal	80000444 <printf>
    800007e6:	40000593          	li	a1,1024
    800007ea:	00001517          	auipc	a0,0x1
    800007ee:	2fe50513          	add	a0,a0,766 # 80001ae8 <etext+0x31c>
    800007f2:	c53ff0ef          	jal	80000444 <printf>
    800007f6:	fe042623          	sw	zero,-20(s0)
    800007fa:	fe042423          	sw	zero,-24(s0)
    800007fe:	a899                	j	80000854 <test_pmm_consistency+0x82>
    80000800:	e99ff0ef          	jal	80000698 <alloc_page>
    80000804:	86aa                	mv	a3,a0
    80000806:	00006717          	auipc	a4,0x6
    8000080a:	b0270713          	add	a4,a4,-1278 # 80006308 <pages>
    8000080e:	fe842783          	lw	a5,-24(s0)
    80000812:	078e                	sll	a5,a5,0x3
    80000814:	97ba                	add	a5,a5,a4
    80000816:	e394                	sd	a3,0(a5)
    80000818:	00006717          	auipc	a4,0x6
    8000081c:	af070713          	add	a4,a4,-1296 # 80006308 <pages>
    80000820:	fe842783          	lw	a5,-24(s0)
    80000824:	078e                	sll	a5,a5,0x3
    80000826:	97ba                	add	a5,a5,a4
    80000828:	639c                	ld	a5,0(a5)
    8000082a:	eb99                	bnez	a5,80000840 <test_pmm_consistency+0x6e>
    8000082c:	fe842783          	lw	a5,-24(s0)
    80000830:	85be                	mv	a1,a5
    80000832:	00001517          	auipc	a0,0x1
    80000836:	2de50513          	add	a0,a0,734 # 80001b10 <etext+0x344>
    8000083a:	c0bff0ef          	jal	80000444 <printf>
    8000083e:	a01d                	j	80000864 <test_pmm_consistency+0x92>
    80000840:	fec42783          	lw	a5,-20(s0)
    80000844:	2785                	addw	a5,a5,1
    80000846:	fef42623          	sw	a5,-20(s0)
    8000084a:	fe842783          	lw	a5,-24(s0)
    8000084e:	2785                	addw	a5,a5,1
    80000850:	fef42423          	sw	a5,-24(s0)
    80000854:	fe842783          	lw	a5,-24(s0)
    80000858:	0007871b          	sext.w	a4,a5
    8000085c:	3ff00793          	li	a5,1023
    80000860:	fae7d0e3          	bge	a5,a4,80000800 <test_pmm_consistency+0x2e>
    80000864:	fec42783          	lw	a5,-20(s0)
    80000868:	85be                	mv	a1,a5
    8000086a:	00001517          	auipc	a0,0x1
    8000086e:	2e650513          	add	a0,a0,742 # 80001b50 <etext+0x384>
    80000872:	bd3ff0ef          	jal	80000444 <printf>
    80000876:	fe042223          	sw	zero,-28(s0)
    8000087a:	a0d5                	j	8000095e <test_pmm_consistency+0x18c>
    8000087c:	00006717          	auipc	a4,0x6
    80000880:	a8c70713          	add	a4,a4,-1396 # 80006308 <pages>
    80000884:	fe442783          	lw	a5,-28(s0)
    80000888:	078e                	sll	a5,a5,0x3
    8000088a:	97ba                	add	a5,a5,a4
    8000088c:	639c                	ld	a5,0(a5)
    8000088e:	873e                	mv	a4,a5
    80000890:	6785                	lui	a5,0x1
    80000892:	17fd                	add	a5,a5,-1 # fff <_entry-0x7ffff001>
    80000894:	8ff9                	and	a5,a5,a4
    80000896:	cb89                	beqz	a5,800008a8 <test_pmm_consistency+0xd6>
    80000898:	00001517          	auipc	a0,0x1
    8000089c:	2e050513          	add	a0,a0,736 # 80001b78 <etext+0x3ac>
    800008a0:	ba5ff0ef          	jal	80000444 <printf>
    800008a4:	0001                	nop
    800008a6:	bffd                	j	800008a4 <test_pmm_consistency+0xd2>
    800008a8:	00006717          	auipc	a4,0x6
    800008ac:	a6070713          	add	a4,a4,-1440 # 80006308 <pages>
    800008b0:	fe442783          	lw	a5,-28(s0)
    800008b4:	078e                	sll	a5,a5,0x3
    800008b6:	97ba                	add	a5,a5,a4
    800008b8:	6398                	ld	a4,0(a5)
    800008ba:	00008797          	auipc	a5,0x8
    800008be:	a5e78793          	add	a5,a5,-1442 # 80008318 <_bss_end>
    800008c2:	02f76163          	bltu	a4,a5,800008e4 <test_pmm_consistency+0x112>
    800008c6:	00006717          	auipc	a4,0x6
    800008ca:	a4270713          	add	a4,a4,-1470 # 80006308 <pages>
    800008ce:	fe442783          	lw	a5,-28(s0)
    800008d2:	078e                	sll	a5,a5,0x3
    800008d4:	97ba                	add	a5,a5,a4
    800008d6:	6398                	ld	a4,0(a5)
    800008d8:	07fff797          	auipc	a5,0x7fff
    800008dc:	72878793          	add	a5,a5,1832 # 88000000 <PHYSTOP>
    800008e0:	00f76a63          	bltu	a4,a5,800008f4 <test_pmm_consistency+0x122>
    800008e4:	00001517          	auipc	a0,0x1
    800008e8:	29450513          	add	a0,a0,660 # 80001b78 <etext+0x3ac>
    800008ec:	b59ff0ef          	jal	80000444 <printf>
    800008f0:	0001                	nop
    800008f2:	bffd                	j	800008f0 <test_pmm_consistency+0x11e>
    800008f4:	fe442783          	lw	a5,-28(s0)
    800008f8:	2785                	addw	a5,a5,1
    800008fa:	fef42023          	sw	a5,-32(s0)
    800008fe:	a091                	j	80000942 <test_pmm_consistency+0x170>
    80000900:	00006717          	auipc	a4,0x6
    80000904:	a0870713          	add	a4,a4,-1528 # 80006308 <pages>
    80000908:	fe442783          	lw	a5,-28(s0)
    8000090c:	078e                	sll	a5,a5,0x3
    8000090e:	97ba                	add	a5,a5,a4
    80000910:	6398                	ld	a4,0(a5)
    80000912:	00006697          	auipc	a3,0x6
    80000916:	9f668693          	add	a3,a3,-1546 # 80006308 <pages>
    8000091a:	fe042783          	lw	a5,-32(s0)
    8000091e:	078e                	sll	a5,a5,0x3
    80000920:	97b6                	add	a5,a5,a3
    80000922:	639c                	ld	a5,0(a5)
    80000924:	00f71a63          	bne	a4,a5,80000938 <test_pmm_consistency+0x166>
    80000928:	00001517          	auipc	a0,0x1
    8000092c:	25050513          	add	a0,a0,592 # 80001b78 <etext+0x3ac>
    80000930:	b15ff0ef          	jal	80000444 <printf>
    80000934:	0001                	nop
    80000936:	bffd                	j	80000934 <test_pmm_consistency+0x162>
    80000938:	fe042783          	lw	a5,-32(s0)
    8000093c:	2785                	addw	a5,a5,1
    8000093e:	fef42023          	sw	a5,-32(s0)
    80000942:	fe042783          	lw	a5,-32(s0)
    80000946:	873e                	mv	a4,a5
    80000948:	fec42783          	lw	a5,-20(s0)
    8000094c:	2701                	sext.w	a4,a4
    8000094e:	2781                	sext.w	a5,a5
    80000950:	faf748e3          	blt	a4,a5,80000900 <test_pmm_consistency+0x12e>
    80000954:	fe442783          	lw	a5,-28(s0)
    80000958:	2785                	addw	a5,a5,1
    8000095a:	fef42223          	sw	a5,-28(s0)
    8000095e:	fe442783          	lw	a5,-28(s0)
    80000962:	873e                	mv	a4,a5
    80000964:	fec42783          	lw	a5,-20(s0)
    80000968:	2701                	sext.w	a4,a4
    8000096a:	2781                	sext.w	a5,a5
    8000096c:	f0f748e3          	blt	a4,a5,8000087c <test_pmm_consistency+0xaa>
    80000970:	00001517          	auipc	a0,0x1
    80000974:	22050513          	add	a0,a0,544 # 80001b90 <etext+0x3c4>
    80000978:	acdff0ef          	jal	80000444 <printf>
    8000097c:	fec42783          	lw	a5,-20(s0)
    80000980:	85be                	mv	a1,a5
    80000982:	00001517          	auipc	a0,0x1
    80000986:	24e50513          	add	a0,a0,590 # 80001bd0 <etext+0x404>
    8000098a:	abbff0ef          	jal	80000444 <printf>
    8000098e:	fc042e23          	sw	zero,-36(s0)
    80000992:	a015                	j	800009b6 <test_pmm_consistency+0x1e4>
    80000994:	00006717          	auipc	a4,0x6
    80000998:	97470713          	add	a4,a4,-1676 # 80006308 <pages>
    8000099c:	fdc42783          	lw	a5,-36(s0)
    800009a0:	078e                	sll	a5,a5,0x3
    800009a2:	97ba                	add	a5,a5,a4
    800009a4:	639c                	ld	a5,0(a5)
    800009a6:	853e                	mv	a0,a5
    800009a8:	c6dff0ef          	jal	80000614 <free_page>
    800009ac:	fdc42783          	lw	a5,-36(s0)
    800009b0:	2785                	addw	a5,a5,1
    800009b2:	fcf42e23          	sw	a5,-36(s0)
    800009b6:	fdc42783          	lw	a5,-36(s0)
    800009ba:	873e                	mv	a4,a5
    800009bc:	fec42783          	lw	a5,-20(s0)
    800009c0:	2701                	sext.w	a4,a4
    800009c2:	2781                	sext.w	a5,a5
    800009c4:	fcf748e3          	blt	a4,a5,80000994 <test_pmm_consistency+0x1c2>
    800009c8:	00001517          	auipc	a0,0x1
    800009cc:	23050513          	add	a0,a0,560 # 80001bf8 <etext+0x42c>
    800009d0:	a75ff0ef          	jal	80000444 <printf>
    800009d4:	00001517          	auipc	a0,0x1
    800009d8:	23c50513          	add	a0,a0,572 # 80001c10 <etext+0x444>
    800009dc:	a69ff0ef          	jal	80000444 <printf>
    800009e0:	cb9ff0ef          	jal	80000698 <alloc_page>
    800009e4:	fca43823          	sd	a0,-48(s0)
    800009e8:	fd043783          	ld	a5,-48(s0)
    800009ec:	eb89                	bnez	a5,800009fe <test_pmm_consistency+0x22c>
    800009ee:	00001517          	auipc	a0,0x1
    800009f2:	18a50513          	add	a0,a0,394 # 80001b78 <etext+0x3ac>
    800009f6:	a4fff0ef          	jal	80000444 <printf>
    800009fa:	0001                	nop
    800009fc:	bffd                	j	800009fa <test_pmm_consistency+0x228>
    800009fe:	fd043583          	ld	a1,-48(s0)
    80000a02:	00001517          	auipc	a0,0x1
    80000a06:	23e50513          	add	a0,a0,574 # 80001c40 <etext+0x474>
    80000a0a:	a3bff0ef          	jal	80000444 <printf>
    80000a0e:	fd043503          	ld	a0,-48(s0)
    80000a12:	c03ff0ef          	jal	80000614 <free_page>
    80000a16:	00001517          	auipc	a0,0x1
    80000a1a:	25a50513          	add	a0,a0,602 # 80001c70 <etext+0x4a4>
    80000a1e:	a27ff0ef          	jal	80000444 <printf>
    80000a22:	0001                	nop
    80000a24:	70a2                	ld	ra,40(sp)
    80000a26:	7402                	ld	s0,32(sp)
    80000a28:	6145                	add	sp,sp,48
    80000a2a:	8082                	ret

0000000080000a2c <walk>:
    80000a2c:	7139                	add	sp,sp,-64
    80000a2e:	fc06                	sd	ra,56(sp)
    80000a30:	f822                	sd	s0,48(sp)
    80000a32:	0080                	add	s0,sp,64
    80000a34:	fca43c23          	sd	a0,-40(s0)
    80000a38:	fcb43823          	sd	a1,-48(s0)
    80000a3c:	87b2                	mv	a5,a2
    80000a3e:	fcf42623          	sw	a5,-52(s0)
    80000a42:	fd043703          	ld	a4,-48(s0)
    80000a46:	57fd                	li	a5,-1
    80000a48:	83e9                	srl	a5,a5,0x1a
    80000a4a:	00e7f463          	bgeu	a5,a4,80000a52 <walk+0x26>
    80000a4e:	4781                	li	a5,0
    80000a50:	a845                	j	80000b00 <walk+0xd4>
    80000a52:	4789                	li	a5,2
    80000a54:	fef42623          	sw	a5,-20(s0)
    80000a58:	a071                	j	80000ae4 <walk+0xb8>
    80000a5a:	fec42783          	lw	a5,-20(s0)
    80000a5e:	873e                	mv	a4,a5
    80000a60:	87ba                	mv	a5,a4
    80000a62:	0037979b          	sllw	a5,a5,0x3
    80000a66:	9fb9                	addw	a5,a5,a4
    80000a68:	2781                	sext.w	a5,a5
    80000a6a:	27b1                	addw	a5,a5,12
    80000a6c:	2781                	sext.w	a5,a5
    80000a6e:	873e                	mv	a4,a5
    80000a70:	fd043783          	ld	a5,-48(s0)
    80000a74:	00e7d7b3          	srl	a5,a5,a4
    80000a78:	1ff7f793          	and	a5,a5,511
    80000a7c:	078e                	sll	a5,a5,0x3
    80000a7e:	fd843703          	ld	a4,-40(s0)
    80000a82:	97ba                	add	a5,a5,a4
    80000a84:	fef43023          	sd	a5,-32(s0)
    80000a88:	fe043783          	ld	a5,-32(s0)
    80000a8c:	639c                	ld	a5,0(a5)
    80000a8e:	8b85                	and	a5,a5,1
    80000a90:	cb89                	beqz	a5,80000aa2 <walk+0x76>
    80000a92:	fe043783          	ld	a5,-32(s0)
    80000a96:	639c                	ld	a5,0(a5)
    80000a98:	83a9                	srl	a5,a5,0xa
    80000a9a:	07b2                	sll	a5,a5,0xc
    80000a9c:	fcf43c23          	sd	a5,-40(s0)
    80000aa0:	a82d                	j	80000ada <walk+0xae>
    80000aa2:	fcc42783          	lw	a5,-52(s0)
    80000aa6:	2781                	sext.w	a5,a5
    80000aa8:	cb81                	beqz	a5,80000ab8 <walk+0x8c>
    80000aaa:	befff0ef          	jal	80000698 <alloc_page>
    80000aae:	fca43c23          	sd	a0,-40(s0)
    80000ab2:	fd843783          	ld	a5,-40(s0)
    80000ab6:	e399                	bnez	a5,80000abc <walk+0x90>
    80000ab8:	4781                	li	a5,0
    80000aba:	a099                	j	80000b00 <walk+0xd4>
    80000abc:	6605                	lui	a2,0x1
    80000abe:	4581                	li	a1,0
    80000ac0:	fd843503          	ld	a0,-40(s0)
    80000ac4:	a3fff0ef          	jal	80000502 <memset>
    80000ac8:	fd843783          	ld	a5,-40(s0)
    80000acc:	83b1                	srl	a5,a5,0xc
    80000ace:	07aa                	sll	a5,a5,0xa
    80000ad0:	0017e713          	or	a4,a5,1
    80000ad4:	fe043783          	ld	a5,-32(s0)
    80000ad8:	e398                	sd	a4,0(a5)
    80000ada:	fec42783          	lw	a5,-20(s0)
    80000ade:	37fd                	addw	a5,a5,-1
    80000ae0:	fef42623          	sw	a5,-20(s0)
    80000ae4:	fec42783          	lw	a5,-20(s0)
    80000ae8:	2781                	sext.w	a5,a5
    80000aea:	f6f048e3          	bgtz	a5,80000a5a <walk+0x2e>
    80000aee:	fd043783          	ld	a5,-48(s0)
    80000af2:	83b1                	srl	a5,a5,0xc
    80000af4:	1ff7f793          	and	a5,a5,511
    80000af8:	078e                	sll	a5,a5,0x3
    80000afa:	fd843703          	ld	a4,-40(s0)
    80000afe:	97ba                	add	a5,a5,a4
    80000b00:	853e                	mv	a0,a5
    80000b02:	70e2                	ld	ra,56(sp)
    80000b04:	7442                	ld	s0,48(sp)
    80000b06:	6121                	add	sp,sp,64
    80000b08:	8082                	ret

0000000080000b0a <map_page>:
    80000b0a:	7139                	add	sp,sp,-64
    80000b0c:	fc06                	sd	ra,56(sp)
    80000b0e:	f822                	sd	s0,48(sp)
    80000b10:	0080                	add	s0,sp,64
    80000b12:	fca43c23          	sd	a0,-40(s0)
    80000b16:	fcb43823          	sd	a1,-48(s0)
    80000b1a:	fcc43423          	sd	a2,-56(s0)
    80000b1e:	87b6                	mv	a5,a3
    80000b20:	fcf42223          	sw	a5,-60(s0)
    80000b24:	fd043703          	ld	a4,-48(s0)
    80000b28:	77fd                	lui	a5,0xfffff
    80000b2a:	8ff9                	and	a5,a5,a4
    80000b2c:	fd043703          	ld	a4,-48(s0)
    80000b30:	00f71a63          	bne	a4,a5,80000b44 <map_page+0x3a>
    80000b34:	fc843703          	ld	a4,-56(s0)
    80000b38:	77fd                	lui	a5,0xfffff
    80000b3a:	8ff9                	and	a5,a5,a4
    80000b3c:	fc843703          	ld	a4,-56(s0)
    80000b40:	00f70463          	beq	a4,a5,80000b48 <map_page+0x3e>
    80000b44:	57fd                	li	a5,-1
    80000b46:	a0a1                	j	80000b8e <map_page+0x84>
    80000b48:	4605                	li	a2,1
    80000b4a:	fd043583          	ld	a1,-48(s0)
    80000b4e:	fd843503          	ld	a0,-40(s0)
    80000b52:	edbff0ef          	jal	80000a2c <walk>
    80000b56:	fea43423          	sd	a0,-24(s0)
    80000b5a:	fe843783          	ld	a5,-24(s0)
    80000b5e:	e399                	bnez	a5,80000b64 <map_page+0x5a>
    80000b60:	57fd                	li	a5,-1
    80000b62:	a035                	j	80000b8e <map_page+0x84>
    80000b64:	fe843783          	ld	a5,-24(s0)
    80000b68:	639c                	ld	a5,0(a5)
    80000b6a:	8b85                	and	a5,a5,1
    80000b6c:	c399                	beqz	a5,80000b72 <map_page+0x68>
    80000b6e:	57fd                	li	a5,-1
    80000b70:	a839                	j	80000b8e <map_page+0x84>
    80000b72:	fc843783          	ld	a5,-56(s0)
    80000b76:	83b1                	srl	a5,a5,0xc
    80000b78:	00a79713          	sll	a4,a5,0xa
    80000b7c:	fc442783          	lw	a5,-60(s0)
    80000b80:	8fd9                	or	a5,a5,a4
    80000b82:	0017e713          	or	a4,a5,1
    80000b86:	fe843783          	ld	a5,-24(s0)
    80000b8a:	e398                	sd	a4,0(a5)
    80000b8c:	4781                	li	a5,0
    80000b8e:	853e                	mv	a0,a5
    80000b90:	70e2                	ld	ra,56(sp)
    80000b92:	7442                	ld	s0,48(sp)
    80000b94:	6121                	add	sp,sp,64
    80000b96:	8082                	ret

0000000080000b98 <create_pagetable>:
    80000b98:	1101                	add	sp,sp,-32
    80000b9a:	ec06                	sd	ra,24(sp)
    80000b9c:	e822                	sd	s0,16(sp)
    80000b9e:	1000                	add	s0,sp,32
    80000ba0:	af9ff0ef          	jal	80000698 <alloc_page>
    80000ba4:	fea43423          	sd	a0,-24(s0)
    80000ba8:	fe843783          	ld	a5,-24(s0)
    80000bac:	c799                	beqz	a5,80000bba <create_pagetable+0x22>
    80000bae:	6605                	lui	a2,0x1
    80000bb0:	4581                	li	a1,0
    80000bb2:	fe843503          	ld	a0,-24(s0)
    80000bb6:	94dff0ef          	jal	80000502 <memset>
    80000bba:	fe843783          	ld	a5,-24(s0)
    80000bbe:	853e                	mv	a0,a5
    80000bc0:	60e2                	ld	ra,24(sp)
    80000bc2:	6442                	ld	s0,16(sp)
    80000bc4:	6105                	add	sp,sp,32
    80000bc6:	8082                	ret

0000000080000bc8 <map_region>:
    80000bc8:	715d                	add	sp,sp,-80
    80000bca:	e486                	sd	ra,72(sp)
    80000bcc:	e0a2                	sd	s0,64(sp)
    80000bce:	0880                	add	s0,sp,80
    80000bd0:	fca43c23          	sd	a0,-40(s0)
    80000bd4:	fcb43823          	sd	a1,-48(s0)
    80000bd8:	fcc43423          	sd	a2,-56(s0)
    80000bdc:	fcd43023          	sd	a3,-64(s0)
    80000be0:	87ba                	mv	a5,a4
    80000be2:	faf42e23          	sw	a5,-68(s0)
    80000be6:	fd043703          	ld	a4,-48(s0)
    80000bea:	6785                	lui	a5,0x1
    80000bec:	17fd                	add	a5,a5,-1 # fff <_entry-0x7ffff001>
    80000bee:	973e                	add	a4,a4,a5
    80000bf0:	77fd                	lui	a5,0xfffff
    80000bf2:	8ff9                	and	a5,a5,a4
    80000bf4:	fcf43823          	sd	a5,-48(s0)
    80000bf8:	fc843703          	ld	a4,-56(s0)
    80000bfc:	6785                	lui	a5,0x1
    80000bfe:	17fd                	add	a5,a5,-1 # fff <_entry-0x7ffff001>
    80000c00:	973e                	add	a4,a4,a5
    80000c02:	77fd                	lui	a5,0xfffff
    80000c04:	8ff9                	and	a5,a5,a4
    80000c06:	fcf43423          	sd	a5,-56(s0)
    80000c0a:	fe043423          	sd	zero,-24(s0)
    80000c0e:	a835                	j	80000c4a <map_region+0x82>
    80000c10:	fd043703          	ld	a4,-48(s0)
    80000c14:	fe843783          	ld	a5,-24(s0)
    80000c18:	00f705b3          	add	a1,a4,a5
    80000c1c:	fc843703          	ld	a4,-56(s0)
    80000c20:	fe843783          	ld	a5,-24(s0)
    80000c24:	97ba                	add	a5,a5,a4
    80000c26:	fbc42703          	lw	a4,-68(s0)
    80000c2a:	86ba                	mv	a3,a4
    80000c2c:	863e                	mv	a2,a5
    80000c2e:	fd843503          	ld	a0,-40(s0)
    80000c32:	ed9ff0ef          	jal	80000b0a <map_page>
    80000c36:	87aa                	mv	a5,a0
    80000c38:	c399                	beqz	a5,80000c3e <map_region+0x76>
    80000c3a:	0001                	nop
    80000c3c:	bffd                	j	80000c3a <map_region+0x72>
    80000c3e:	fe843703          	ld	a4,-24(s0)
    80000c42:	6785                	lui	a5,0x1
    80000c44:	97ba                	add	a5,a5,a4
    80000c46:	fef43423          	sd	a5,-24(s0)
    80000c4a:	fe843703          	ld	a4,-24(s0)
    80000c4e:	fc043783          	ld	a5,-64(s0)
    80000c52:	faf76fe3          	bltu	a4,a5,80000c10 <map_region+0x48>
    80000c56:	0001                	nop
    80000c58:	0001                	nop
    80000c5a:	60a6                	ld	ra,72(sp)
    80000c5c:	6406                	ld	s0,64(sp)
    80000c5e:	6161                	add	sp,sp,80
    80000c60:	8082                	ret

0000000080000c62 <kvminit>:
    80000c62:	1141                	add	sp,sp,-16
    80000c64:	e406                	sd	ra,8(sp)
    80000c66:	e022                	sd	s0,0(sp)
    80000c68:	0800                	add	s0,sp,16
    80000c6a:	f2fff0ef          	jal	80000b98 <create_pagetable>
    80000c6e:	872a                	mv	a4,a0
    80000c70:	00007797          	auipc	a5,0x7
    80000c74:	69878793          	add	a5,a5,1688 # 80008308 <kernel_pagetable>
    80000c78:	e398                	sd	a4,0(a5)
    80000c7a:	00001517          	auipc	a0,0x1
    80000c7e:	01e50513          	add	a0,a0,30 # 80001c98 <etext+0x4cc>
    80000c82:	fc2ff0ef          	jal	80000444 <printf>
    80000c86:	00007797          	auipc	a5,0x7
    80000c8a:	68278793          	add	a5,a5,1666 # 80008308 <kernel_pagetable>
    80000c8e:	639c                	ld	a5,0(a5)
    80000c90:	4699                	li	a3,6
    80000c92:	10000637          	lui	a2,0x10000
    80000c96:	100005b7          	lui	a1,0x10000
    80000c9a:	853e                	mv	a0,a5
    80000c9c:	e6fff0ef          	jal	80000b0a <map_page>
    80000ca0:	00001517          	auipc	a0,0x1
    80000ca4:	01050513          	add	a0,a0,16 # 80001cb0 <etext+0x4e4>
    80000ca8:	f9cff0ef          	jal	80000444 <printf>
    80000cac:	00007797          	auipc	a5,0x7
    80000cb0:	65c78793          	add	a5,a5,1628 # 80008308 <kernel_pagetable>
    80000cb4:	6388                	ld	a0,0(a5)
    80000cb6:	00001717          	auipc	a4,0x1
    80000cba:	b1670713          	add	a4,a4,-1258 # 800017cc <etext>
    80000cbe:	800007b7          	lui	a5,0x80000
    80000cc2:	97ba                	add	a5,a5,a4
    80000cc4:	4729                	li	a4,10
    80000cc6:	86be                	mv	a3,a5
    80000cc8:	4785                	li	a5,1
    80000cca:	01f79613          	sll	a2,a5,0x1f
    80000cce:	4785                	li	a5,1
    80000cd0:	01f79593          	sll	a1,a5,0x1f
    80000cd4:	ef5ff0ef          	jal	80000bc8 <map_region>
    80000cd8:	00001517          	auipc	a0,0x1
    80000cdc:	ff850513          	add	a0,a0,-8 # 80001cd0 <etext+0x504>
    80000ce0:	f64ff0ef          	jal	80000444 <printf>
    80000ce4:	00007797          	auipc	a5,0x7
    80000ce8:	62478793          	add	a5,a5,1572 # 80008308 <kernel_pagetable>
    80000cec:	6388                	ld	a0,0(a5)
    80000cee:	00001597          	auipc	a1,0x1
    80000cf2:	ade58593          	add	a1,a1,-1314 # 800017cc <etext>
    80000cf6:	00001617          	auipc	a2,0x1
    80000cfa:	ad660613          	add	a2,a2,-1322 # 800017cc <etext>
    80000cfe:	07fff717          	auipc	a4,0x7fff
    80000d02:	30270713          	add	a4,a4,770 # 88000000 <PHYSTOP>
    80000d06:	00001797          	auipc	a5,0x1
    80000d0a:	ac678793          	add	a5,a5,-1338 # 800017cc <etext>
    80000d0e:	40f707b3          	sub	a5,a4,a5
    80000d12:	4719                	li	a4,6
    80000d14:	86be                	mv	a3,a5
    80000d16:	eb3ff0ef          	jal	80000bc8 <map_region>
    80000d1a:	0001                	nop
    80000d1c:	60a2                	ld	ra,8(sp)
    80000d1e:	6402                	ld	s0,0(sp)
    80000d20:	0141                	add	sp,sp,16
    80000d22:	8082                	ret

0000000080000d24 <kvminithart>:
    80000d24:	1101                	add	sp,sp,-32
    80000d26:	ec06                	sd	ra,24(sp)
    80000d28:	e822                	sd	s0,16(sp)
    80000d2a:	1000                	add	s0,sp,32
    80000d2c:	00007797          	auipc	a5,0x7
    80000d30:	5dc78793          	add	a5,a5,1500 # 80008308 <kernel_pagetable>
    80000d34:	639c                	ld	a5,0(a5)
    80000d36:	00c7d713          	srl	a4,a5,0xc
    80000d3a:	57fd                	li	a5,-1
    80000d3c:	17fe                	sll	a5,a5,0x3f
    80000d3e:	8fd9                	or	a5,a5,a4
    80000d40:	fef43423          	sd	a5,-24(s0)
    80000d44:	fe843783          	ld	a5,-24(s0)
    80000d48:	18079073          	csrw	satp,a5
    80000d4c:	12000073          	sfence.vma
    80000d50:	00001517          	auipc	a0,0x1
    80000d54:	fb050513          	add	a0,a0,-80 # 80001d00 <etext+0x534>
    80000d58:	eecff0ef          	jal	80000444 <printf>
    80000d5c:	0001                	nop
    80000d5e:	60e2                	ld	ra,24(sp)
    80000d60:	6442                	ld	s0,16(sp)
    80000d62:	6105                	add	sp,sp,32
    80000d64:	8082                	ret

0000000080000d66 <test_pagetable>:
    80000d66:	7139                	add	sp,sp,-64
    80000d68:	fc06                	sd	ra,56(sp)
    80000d6a:	f822                	sd	s0,48(sp)
    80000d6c:	0080                	add	s0,sp,64
    80000d6e:	00001517          	auipc	a0,0x1
    80000d72:	faa50513          	add	a0,a0,-86 # 80001d18 <etext+0x54c>
    80000d76:	eceff0ef          	jal	80000444 <printf>
    80000d7a:	e1fff0ef          	jal	80000b98 <create_pagetable>
    80000d7e:	fea43423          	sd	a0,-24(s0)
    80000d82:	fe843783          	ld	a5,-24(s0)
    80000d86:	e39d                	bnez	a5,80000dac <test_pagetable+0x46>
    80000d88:	07400693          	li	a3,116
    80000d8c:	00001617          	auipc	a2,0x1
    80000d90:	fbc60613          	add	a2,a2,-68 # 80001d48 <etext+0x57c>
    80000d94:	00001597          	auipc	a1,0x1
    80000d98:	fc458593          	add	a1,a1,-60 # 80001d58 <etext+0x58c>
    80000d9c:	00001517          	auipc	a0,0x1
    80000da0:	fc450513          	add	a0,a0,-60 # 80001d60 <etext+0x594>
    80000da4:	ea0ff0ef          	jal	80000444 <printf>
    80000da8:	0001                	nop
    80000daa:	bffd                	j	80000da8 <test_pagetable+0x42>
    80000dac:	fe843583          	ld	a1,-24(s0)
    80000db0:	00001517          	auipc	a0,0x1
    80000db4:	fd850513          	add	a0,a0,-40 # 80001d88 <etext+0x5bc>
    80000db8:	e8cff0ef          	jal	80000444 <printf>
    80000dbc:	001007b7          	lui	a5,0x100
    80000dc0:	fef43023          	sd	a5,-32(s0)
    80000dc4:	8d5ff0ef          	jal	80000698 <alloc_page>
    80000dc8:	fca43c23          	sd	a0,-40(s0)
    80000dcc:	fd843783          	ld	a5,-40(s0)
    80000dd0:	e39d                	bnez	a5,80000df6 <test_pagetable+0x90>
    80000dd2:	07a00693          	li	a3,122
    80000dd6:	00001617          	auipc	a2,0x1
    80000dda:	f7260613          	add	a2,a2,-142 # 80001d48 <etext+0x57c>
    80000dde:	00001597          	auipc	a1,0x1
    80000de2:	fd258593          	add	a1,a1,-46 # 80001db0 <etext+0x5e4>
    80000de6:	00001517          	auipc	a0,0x1
    80000dea:	f7a50513          	add	a0,a0,-134 # 80001d60 <etext+0x594>
    80000dee:	e56ff0ef          	jal	80000444 <printf>
    80000df2:	0001                	nop
    80000df4:	bffd                	j	80000df2 <test_pagetable+0x8c>
    80000df6:	fd843783          	ld	a5,-40(s0)
    80000dfa:	fcf43823          	sd	a5,-48(s0)
    80000dfe:	fd043783          	ld	a5,-48(s0)
    80000e02:	863e                	mv	a2,a5
    80000e04:	fe043583          	ld	a1,-32(s0)
    80000e08:	00001517          	auipc	a0,0x1
    80000e0c:	fb850513          	add	a0,a0,-72 # 80001dc0 <etext+0x5f4>
    80000e10:	e34ff0ef          	jal	80000444 <printf>
    80000e14:	4699                	li	a3,6
    80000e16:	fd043603          	ld	a2,-48(s0)
    80000e1a:	fe043583          	ld	a1,-32(s0)
    80000e1e:	fe843503          	ld	a0,-24(s0)
    80000e22:	ce9ff0ef          	jal	80000b0a <map_page>
    80000e26:	87aa                	mv	a5,a0
    80000e28:	fcf42623          	sw	a5,-52(s0)
    80000e2c:	fcc42783          	lw	a5,-52(s0)
    80000e30:	2781                	sext.w	a5,a5
    80000e32:	c39d                	beqz	a5,80000e58 <test_pagetable+0xf2>
    80000e34:	07f00693          	li	a3,127
    80000e38:	00001617          	auipc	a2,0x1
    80000e3c:	f1060613          	add	a2,a2,-240 # 80001d48 <etext+0x57c>
    80000e40:	00001597          	auipc	a1,0x1
    80000e44:	fa058593          	add	a1,a1,-96 # 80001de0 <etext+0x614>
    80000e48:	00001517          	auipc	a0,0x1
    80000e4c:	f1850513          	add	a0,a0,-232 # 80001d60 <etext+0x594>
    80000e50:	df4ff0ef          	jal	80000444 <printf>
    80000e54:	0001                	nop
    80000e56:	bffd                	j	80000e54 <test_pagetable+0xee>
    80000e58:	4601                	li	a2,0
    80000e5a:	fe043583          	ld	a1,-32(s0)
    80000e5e:	fe843503          	ld	a0,-24(s0)
    80000e62:	bcbff0ef          	jal	80000a2c <walk>
    80000e66:	fca43023          	sd	a0,-64(s0)
    80000e6a:	fc043783          	ld	a5,-64(s0)
    80000e6e:	e39d                	bnez	a5,80000e94 <test_pagetable+0x12e>
    80000e70:	08300693          	li	a3,131
    80000e74:	00001617          	auipc	a2,0x1
    80000e78:	ed460613          	add	a2,a2,-300 # 80001d48 <etext+0x57c>
    80000e7c:	00001597          	auipc	a1,0x1
    80000e80:	f7458593          	add	a1,a1,-140 # 80001df0 <etext+0x624>
    80000e84:	00001517          	auipc	a0,0x1
    80000e88:	edc50513          	add	a0,a0,-292 # 80001d60 <etext+0x594>
    80000e8c:	db8ff0ef          	jal	80000444 <printf>
    80000e90:	0001                	nop
    80000e92:	bffd                	j	80000e90 <test_pagetable+0x12a>
    80000e94:	fc043783          	ld	a5,-64(s0)
    80000e98:	639c                	ld	a5,0(a5)
    80000e9a:	8b85                	and	a5,a5,1
    80000e9c:	e39d                	bnez	a5,80000ec2 <test_pagetable+0x15c>
    80000e9e:	08400693          	li	a3,132
    80000ea2:	00001617          	auipc	a2,0x1
    80000ea6:	ea660613          	add	a2,a2,-346 # 80001d48 <etext+0x57c>
    80000eaa:	00001597          	auipc	a1,0x1
    80000eae:	f5658593          	add	a1,a1,-170 # 80001e00 <etext+0x634>
    80000eb2:	00001517          	auipc	a0,0x1
    80000eb6:	eae50513          	add	a0,a0,-338 # 80001d60 <etext+0x594>
    80000eba:	d8aff0ef          	jal	80000444 <printf>
    80000ebe:	0001                	nop
    80000ec0:	bffd                	j	80000ebe <test_pagetable+0x158>
    80000ec2:	fc043783          	ld	a5,-64(s0)
    80000ec6:	639c                	ld	a5,0(a5)
    80000ec8:	0067f713          	and	a4,a5,6
    80000ecc:	4799                	li	a5,6
    80000ece:	02f70463          	beq	a4,a5,80000ef6 <test_pagetable+0x190>
    80000ed2:	08500693          	li	a3,133
    80000ed6:	00001617          	auipc	a2,0x1
    80000eda:	e7260613          	add	a2,a2,-398 # 80001d48 <etext+0x57c>
    80000ede:	00001597          	auipc	a1,0x1
    80000ee2:	f3258593          	add	a1,a1,-206 # 80001e10 <etext+0x644>
    80000ee6:	00001517          	auipc	a0,0x1
    80000eea:	e7a50513          	add	a0,a0,-390 # 80001d60 <etext+0x594>
    80000eee:	d56ff0ef          	jal	80000444 <printf>
    80000ef2:	0001                	nop
    80000ef4:	bffd                	j	80000ef2 <test_pagetable+0x18c>
    80000ef6:	fc043783          	ld	a5,-64(s0)
    80000efa:	639c                	ld	a5,0(a5)
    80000efc:	8ba1                	and	a5,a5,8
    80000efe:	c39d                	beqz	a5,80000f24 <test_pagetable+0x1be>
    80000f00:	08600693          	li	a3,134
    80000f04:	00001617          	auipc	a2,0x1
    80000f08:	e4460613          	add	a2,a2,-444 # 80001d48 <etext+0x57c>
    80000f0c:	00001597          	auipc	a1,0x1
    80000f10:	f3458593          	add	a1,a1,-204 # 80001e40 <etext+0x674>
    80000f14:	00001517          	auipc	a0,0x1
    80000f18:	e4c50513          	add	a0,a0,-436 # 80001d60 <etext+0x594>
    80000f1c:	d28ff0ef          	jal	80000444 <printf>
    80000f20:	0001                	nop
    80000f22:	bffd                	j	80000f20 <test_pagetable+0x1ba>
    80000f24:	fc043783          	ld	a5,-64(s0)
    80000f28:	639c                	ld	a5,0(a5)
    80000f2a:	83a9                	srl	a5,a5,0xa
    80000f2c:	07b2                	sll	a5,a5,0xc
    80000f2e:	fd043703          	ld	a4,-48(s0)
    80000f32:	02f70463          	beq	a4,a5,80000f5a <test_pagetable+0x1f4>
    80000f36:	08700693          	li	a3,135
    80000f3a:	00001617          	auipc	a2,0x1
    80000f3e:	e0e60613          	add	a2,a2,-498 # 80001d48 <etext+0x57c>
    80000f42:	00001597          	auipc	a1,0x1
    80000f46:	f0e58593          	add	a1,a1,-242 # 80001e50 <etext+0x684>
    80000f4a:	00001517          	auipc	a0,0x1
    80000f4e:	e1650513          	add	a0,a0,-490 # 80001d60 <etext+0x594>
    80000f52:	cf2ff0ef          	jal	80000444 <printf>
    80000f56:	0001                	nop
    80000f58:	bffd                	j	80000f56 <test_pagetable+0x1f0>
    80000f5a:	00001517          	auipc	a0,0x1
    80000f5e:	f0e50513          	add	a0,a0,-242 # 80001e68 <etext+0x69c>
    80000f62:	ce2ff0ef          	jal	80000444 <printf>
    80000f66:	fd843503          	ld	a0,-40(s0)
    80000f6a:	eaaff0ef          	jal	80000614 <free_page>
    80000f6e:	00001517          	auipc	a0,0x1
    80000f72:	f4a50513          	add	a0,a0,-182 # 80001eb8 <etext+0x6ec>
    80000f76:	cceff0ef          	jal	80000444 <printf>
    80000f7a:	0001                	nop
    80000f7c:	70e2                	ld	ra,56(sp)
    80000f7e:	7442                	ld	s0,48(sp)
    80000f80:	6121                	add	sp,sp,64
    80000f82:	8082                	ret

0000000080000f84 <test_page_table_remap>:
    80000f84:	7139                	add	sp,sp,-64
    80000f86:	fc06                	sd	ra,56(sp)
    80000f88:	f822                	sd	s0,48(sp)
    80000f8a:	0080                	add	s0,sp,64
    80000f8c:	00001517          	auipc	a0,0x1
    80000f90:	f5450513          	add	a0,a0,-172 # 80001ee0 <etext+0x714>
    80000f94:	cb0ff0ef          	jal	80000444 <printf>
    80000f98:	c01ff0ef          	jal	80000b98 <create_pagetable>
    80000f9c:	fea43423          	sd	a0,-24(s0)
    80000fa0:	fe843783          	ld	a5,-24(s0)
    80000fa4:	e39d                	bnez	a5,80000fca <test_page_table_remap+0x46>
    80000fa6:	09800693          	li	a3,152
    80000faa:	00001617          	auipc	a2,0x1
    80000fae:	d9e60613          	add	a2,a2,-610 # 80001d48 <etext+0x57c>
    80000fb2:	00001597          	auipc	a1,0x1
    80000fb6:	da658593          	add	a1,a1,-602 # 80001d58 <etext+0x58c>
    80000fba:	00001517          	auipc	a0,0x1
    80000fbe:	da650513          	add	a0,a0,-602 # 80001d60 <etext+0x594>
    80000fc2:	c82ff0ef          	jal	80000444 <printf>
    80000fc6:	0001                	nop
    80000fc8:	bffd                	j	80000fc6 <test_page_table_remap+0x42>
    80000fca:	002007b7          	lui	a5,0x200
    80000fce:	fef43023          	sd	a5,-32(s0)
    80000fd2:	ec6ff0ef          	jal	80000698 <alloc_page>
    80000fd6:	fca43c23          	sd	a0,-40(s0)
    80000fda:	fd843783          	ld	a5,-40(s0)
    80000fde:	e39d                	bnez	a5,80001004 <test_page_table_remap+0x80>
    80000fe0:	09c00693          	li	a3,156
    80000fe4:	00001617          	auipc	a2,0x1
    80000fe8:	d6460613          	add	a2,a2,-668 # 80001d48 <etext+0x57c>
    80000fec:	00001597          	auipc	a1,0x1
    80000ff0:	dc458593          	add	a1,a1,-572 # 80001db0 <etext+0x5e4>
    80000ff4:	00001517          	auipc	a0,0x1
    80000ff8:	d6c50513          	add	a0,a0,-660 # 80001d60 <etext+0x594>
    80000ffc:	c48ff0ef          	jal	80000444 <printf>
    80001000:	0001                	nop
    80001002:	bffd                	j	80001000 <test_page_table_remap+0x7c>
    80001004:	fd843783          	ld	a5,-40(s0)
    80001008:	fcf43823          	sd	a5,-48(s0)
    8000100c:	00001517          	auipc	a0,0x1
    80001010:	f0450513          	add	a0,a0,-252 # 80001f10 <etext+0x744>
    80001014:	c30ff0ef          	jal	80000444 <printf>
    80001018:	4699                	li	a3,6
    8000101a:	fd043603          	ld	a2,-48(s0)
    8000101e:	fe043583          	ld	a1,-32(s0)
    80001022:	fe843503          	ld	a0,-24(s0)
    80001026:	ae5ff0ef          	jal	80000b0a <map_page>
    8000102a:	87aa                	mv	a5,a0
    8000102c:	c39d                	beqz	a5,80001052 <test_page_table_remap+0xce>
    8000102e:	0a100693          	li	a3,161
    80001032:	00001617          	auipc	a2,0x1
    80001036:	d1660613          	add	a2,a2,-746 # 80001d48 <etext+0x57c>
    8000103a:	00001597          	auipc	a1,0x1
    8000103e:	ef658593          	add	a1,a1,-266 # 80001f30 <etext+0x764>
    80001042:	00001517          	auipc	a0,0x1
    80001046:	d1e50513          	add	a0,a0,-738 # 80001d60 <etext+0x594>
    8000104a:	bfaff0ef          	jal	80000444 <printf>
    8000104e:	0001                	nop
    80001050:	bffd                	j	8000104e <test_page_table_remap+0xca>
    80001052:	00001517          	auipc	a0,0x1
    80001056:	f0e50513          	add	a0,a0,-242 # 80001f60 <etext+0x794>
    8000105a:	beaff0ef          	jal	80000444 <printf>
    8000105e:	46a9                	li	a3,10
    80001060:	fd043603          	ld	a2,-48(s0)
    80001064:	fe043583          	ld	a1,-32(s0)
    80001068:	fe843503          	ld	a0,-24(s0)
    8000106c:	a9fff0ef          	jal	80000b0a <map_page>
    80001070:	87aa                	mv	a5,a0
    80001072:	873e                	mv	a4,a5
    80001074:	57fd                	li	a5,-1
    80001076:	02f70463          	beq	a4,a5,8000109e <test_page_table_remap+0x11a>
    8000107a:	0a600693          	li	a3,166
    8000107e:	00001617          	auipc	a2,0x1
    80001082:	cca60613          	add	a2,a2,-822 # 80001d48 <etext+0x57c>
    80001086:	00001597          	auipc	a1,0x1
    8000108a:	efa58593          	add	a1,a1,-262 # 80001f80 <etext+0x7b4>
    8000108e:	00001517          	auipc	a0,0x1
    80001092:	cd250513          	add	a0,a0,-814 # 80001d60 <etext+0x594>
    80001096:	baeff0ef          	jal	80000444 <printf>
    8000109a:	0001                	nop
    8000109c:	bffd                	j	8000109a <test_page_table_remap+0x116>
    8000109e:	00001517          	auipc	a0,0x1
    800010a2:	f1250513          	add	a0,a0,-238 # 80001fb0 <etext+0x7e4>
    800010a6:	b9eff0ef          	jal	80000444 <printf>
    800010aa:	00001517          	auipc	a0,0x1
    800010ae:	f2e50513          	add	a0,a0,-210 # 80001fd8 <etext+0x80c>
    800010b2:	b92ff0ef          	jal	80000444 <printf>
    800010b6:	4601                	li	a2,0
    800010b8:	fe043583          	ld	a1,-32(s0)
    800010bc:	fe843503          	ld	a0,-24(s0)
    800010c0:	96dff0ef          	jal	80000a2c <walk>
    800010c4:	fca43423          	sd	a0,-56(s0)
    800010c8:	fc843783          	ld	a5,-56(s0)
    800010cc:	c791                	beqz	a5,800010d8 <test_page_table_remap+0x154>
    800010ce:	fc843783          	ld	a5,-56(s0)
    800010d2:	639c                	ld	a5,0(a5)
    800010d4:	8b85                	and	a5,a5,1
    800010d6:	e39d                	bnez	a5,800010fc <test_page_table_remap+0x178>
    800010d8:	0ac00693          	li	a3,172
    800010dc:	00001617          	auipc	a2,0x1
    800010e0:	c6c60613          	add	a2,a2,-916 # 80001d48 <etext+0x57c>
    800010e4:	00001597          	auipc	a1,0x1
    800010e8:	f1458593          	add	a1,a1,-236 # 80001ff8 <etext+0x82c>
    800010ec:	00001517          	auipc	a0,0x1
    800010f0:	c7450513          	add	a0,a0,-908 # 80001d60 <etext+0x594>
    800010f4:	b50ff0ef          	jal	80000444 <printf>
    800010f8:	0001                	nop
    800010fa:	bffd                	j	800010f8 <test_page_table_remap+0x174>
    800010fc:	fc843783          	ld	a5,-56(s0)
    80001100:	639c                	ld	a5,0(a5)
    80001102:	0067f713          	and	a4,a5,6
    80001106:	4799                	li	a5,6
    80001108:	02f70463          	beq	a4,a5,80001130 <test_page_table_remap+0x1ac>
    8000110c:	0ae00693          	li	a3,174
    80001110:	00001617          	auipc	a2,0x1
    80001114:	c3860613          	add	a2,a2,-968 # 80001d48 <etext+0x57c>
    80001118:	00001597          	auipc	a1,0x1
    8000111c:	cf858593          	add	a1,a1,-776 # 80001e10 <etext+0x644>
    80001120:	00001517          	auipc	a0,0x1
    80001124:	c4050513          	add	a0,a0,-960 # 80001d60 <etext+0x594>
    80001128:	b1cff0ef          	jal	80000444 <printf>
    8000112c:	0001                	nop
    8000112e:	bffd                	j	8000112c <test_page_table_remap+0x1a8>
    80001130:	fc843783          	ld	a5,-56(s0)
    80001134:	639c                	ld	a5,0(a5)
    80001136:	8ba1                	and	a5,a5,8
    80001138:	c39d                	beqz	a5,8000115e <test_page_table_remap+0x1da>
    8000113a:	0af00693          	li	a3,175
    8000113e:	00001617          	auipc	a2,0x1
    80001142:	c0a60613          	add	a2,a2,-1014 # 80001d48 <etext+0x57c>
    80001146:	00001597          	auipc	a1,0x1
    8000114a:	cfa58593          	add	a1,a1,-774 # 80001e40 <etext+0x674>
    8000114e:	00001517          	auipc	a0,0x1
    80001152:	c1250513          	add	a0,a0,-1006 # 80001d60 <etext+0x594>
    80001156:	aeeff0ef          	jal	80000444 <printf>
    8000115a:	0001                	nop
    8000115c:	bffd                	j	8000115a <test_page_table_remap+0x1d6>
    8000115e:	00001517          	auipc	a0,0x1
    80001162:	eba50513          	add	a0,a0,-326 # 80002018 <etext+0x84c>
    80001166:	adeff0ef          	jal	80000444 <printf>
    8000116a:	fc843783          	ld	a5,-56(s0)
    8000116e:	0007b023          	sd	zero,0(a5) # 200000 <_entry-0x7fe00000>
    80001172:	fd843503          	ld	a0,-40(s0)
    80001176:	c9eff0ef          	jal	80000614 <free_page>
    8000117a:	fe843503          	ld	a0,-24(s0)
    8000117e:	c96ff0ef          	jal	80000614 <free_page>
    80001182:	00001517          	auipc	a0,0x1
    80001186:	ebe50513          	add	a0,a0,-322 # 80002040 <etext+0x874>
    8000118a:	abaff0ef          	jal	80000444 <printf>
    8000118e:	0001                	nop
    80001190:	70e2                	ld	ra,56(sp)
    80001192:	7442                	ld	s0,48(sp)
    80001194:	6121                	add	sp,sp,64
    80001196:	8082                	ret

0000000080001198 <test_kernel_mapping>:
    80001198:	7179                	add	sp,sp,-48
    8000119a:	f406                	sd	ra,40(sp)
    8000119c:	f022                	sd	s0,32(sp)
    8000119e:	1800                	add	s0,sp,48
    800011a0:	00001517          	auipc	a0,0x1
    800011a4:	ed050513          	add	a0,a0,-304 # 80002070 <etext+0x8a4>
    800011a8:	a9cff0ef          	jal	80000444 <printf>
    800011ac:	00001517          	auipc	a0,0x1
    800011b0:	efc50513          	add	a0,a0,-260 # 800020a8 <etext+0x8dc>
    800011b4:	a90ff0ef          	jal	80000444 <printf>
    800011b8:	00007797          	auipc	a5,0x7
    800011bc:	15078793          	add	a5,a5,336 # 80008308 <kernel_pagetable>
    800011c0:	6398                	ld	a4,0(a5)
    800011c2:	4601                	li	a2,0
    800011c4:	4785                	li	a5,1
    800011c6:	01f79593          	sll	a1,a5,0x1f
    800011ca:	853a                	mv	a0,a4
    800011cc:	861ff0ef          	jal	80000a2c <walk>
    800011d0:	fea43423          	sd	a0,-24(s0)
    800011d4:	fe843783          	ld	a5,-24(s0)
    800011d8:	c791                	beqz	a5,800011e4 <test_kernel_mapping+0x4c>
    800011da:	fe843783          	ld	a5,-24(s0)
    800011de:	639c                	ld	a5,0(a5)
    800011e0:	8b85                	and	a5,a5,1
    800011e2:	e39d                	bnez	a5,80001208 <test_kernel_mapping+0x70>
    800011e4:	0c800693          	li	a3,200
    800011e8:	00001617          	auipc	a2,0x1
    800011ec:	b6060613          	add	a2,a2,-1184 # 80001d48 <etext+0x57c>
    800011f0:	00001597          	auipc	a1,0x1
    800011f4:	e0858593          	add	a1,a1,-504 # 80001ff8 <etext+0x82c>
    800011f8:	00001517          	auipc	a0,0x1
    800011fc:	b6850513          	add	a0,a0,-1176 # 80001d60 <etext+0x594>
    80001200:	a44ff0ef          	jal	80000444 <printf>
    80001204:	0001                	nop
    80001206:	bffd                	j	80001204 <test_kernel_mapping+0x6c>
    80001208:	fe843783          	ld	a5,-24(s0)
    8000120c:	639c                	ld	a5,0(a5)
    8000120e:	8ba1                	and	a5,a5,8
    80001210:	e39d                	bnez	a5,80001236 <test_kernel_mapping+0x9e>
    80001212:	0c900693          	li	a3,201
    80001216:	00001617          	auipc	a2,0x1
    8000121a:	b3260613          	add	a2,a2,-1230 # 80001d48 <etext+0x57c>
    8000121e:	00001597          	auipc	a1,0x1
    80001222:	ec258593          	add	a1,a1,-318 # 800020e0 <etext+0x914>
    80001226:	00001517          	auipc	a0,0x1
    8000122a:	b3a50513          	add	a0,a0,-1222 # 80001d60 <etext+0x594>
    8000122e:	a16ff0ef          	jal	80000444 <printf>
    80001232:	0001                	nop
    80001234:	bffd                	j	80001232 <test_kernel_mapping+0x9a>
    80001236:	fe843783          	ld	a5,-24(s0)
    8000123a:	639c                	ld	a5,0(a5)
    8000123c:	8b91                	and	a5,a5,4
    8000123e:	c39d                	beqz	a5,80001264 <test_kernel_mapping+0xcc>
    80001240:	0ca00693          	li	a3,202
    80001244:	00001617          	auipc	a2,0x1
    80001248:	b0460613          	add	a2,a2,-1276 # 80001d48 <etext+0x57c>
    8000124c:	00001597          	auipc	a1,0x1
    80001250:	ea458593          	add	a1,a1,-348 # 800020f0 <etext+0x924>
    80001254:	00001517          	auipc	a0,0x1
    80001258:	b0c50513          	add	a0,a0,-1268 # 80001d60 <etext+0x594>
    8000125c:	9e8ff0ef          	jal	80000444 <printf>
    80001260:	0001                	nop
    80001262:	bffd                	j	80001260 <test_kernel_mapping+0xc8>
    80001264:	fe843783          	ld	a5,-24(s0)
    80001268:	639c                	ld	a5,0(a5)
    8000126a:	83a9                	srl	a5,a5,0xa
    8000126c:	07b2                	sll	a5,a5,0xc
    8000126e:	fef43023          	sd	a5,-32(s0)
    80001272:	fe043703          	ld	a4,-32(s0)
    80001276:	4785                	li	a5,1
    80001278:	07fe                	sll	a5,a5,0x1f
    8000127a:	02f70463          	beq	a4,a5,800012a2 <test_kernel_mapping+0x10a>
    8000127e:	0cc00693          	li	a3,204
    80001282:	00001617          	auipc	a2,0x1
    80001286:	ac660613          	add	a2,a2,-1338 # 80001d48 <etext+0x57c>
    8000128a:	00001597          	auipc	a1,0x1
    8000128e:	e7658593          	add	a1,a1,-394 # 80002100 <etext+0x934>
    80001292:	00001517          	auipc	a0,0x1
    80001296:	ace50513          	add	a0,a0,-1330 # 80001d60 <etext+0x594>
    8000129a:	9aaff0ef          	jal	80000444 <printf>
    8000129e:	0001                	nop
    800012a0:	bffd                	j	8000129e <test_kernel_mapping+0x106>
    800012a2:	00001517          	auipc	a0,0x1
    800012a6:	e6e50513          	add	a0,a0,-402 # 80002110 <etext+0x944>
    800012aa:	99aff0ef          	jal	80000444 <printf>
    800012ae:	00001517          	auipc	a0,0x1
    800012b2:	e8250513          	add	a0,a0,-382 # 80002130 <etext+0x964>
    800012b6:	98eff0ef          	jal	80000444 <printf>
    800012ba:	00000717          	auipc	a4,0x0
    800012be:	51270713          	add	a4,a4,1298 # 800017cc <etext>
    800012c2:	6785                	lui	a5,0x1
    800012c4:	17fd                	add	a5,a5,-1 # fff <_entry-0x7ffff001>
    800012c6:	973e                	add	a4,a4,a5
    800012c8:	77fd                	lui	a5,0xfffff
    800012ca:	8ff9                	and	a5,a5,a4
    800012cc:	fcf43c23          	sd	a5,-40(s0)
    800012d0:	00007797          	auipc	a5,0x7
    800012d4:	03878793          	add	a5,a5,56 # 80008308 <kernel_pagetable>
    800012d8:	639c                	ld	a5,0(a5)
    800012da:	4601                	li	a2,0
    800012dc:	fd843583          	ld	a1,-40(s0)
    800012e0:	853e                	mv	a0,a5
    800012e2:	f4aff0ef          	jal	80000a2c <walk>
    800012e6:	fea43423          	sd	a0,-24(s0)
    800012ea:	fe843783          	ld	a5,-24(s0)
    800012ee:	c791                	beqz	a5,800012fa <test_kernel_mapping+0x162>
    800012f0:	fe843783          	ld	a5,-24(s0)
    800012f4:	639c                	ld	a5,0(a5)
    800012f6:	8b85                	and	a5,a5,1
    800012f8:	e39d                	bnez	a5,8000131e <test_kernel_mapping+0x186>
    800012fa:	0d300693          	li	a3,211
    800012fe:	00001617          	auipc	a2,0x1
    80001302:	a4a60613          	add	a2,a2,-1462 # 80001d48 <etext+0x57c>
    80001306:	00001597          	auipc	a1,0x1
    8000130a:	cf258593          	add	a1,a1,-782 # 80001ff8 <etext+0x82c>
    8000130e:	00001517          	auipc	a0,0x1
    80001312:	a5250513          	add	a0,a0,-1454 # 80001d60 <etext+0x594>
    80001316:	92eff0ef          	jal	80000444 <printf>
    8000131a:	0001                	nop
    8000131c:	bffd                	j	8000131a <test_kernel_mapping+0x182>
    8000131e:	fe843783          	ld	a5,-24(s0)
    80001322:	639c                	ld	a5,0(a5)
    80001324:	8ba1                	and	a5,a5,8
    80001326:	c39d                	beqz	a5,8000134c <test_kernel_mapping+0x1b4>
    80001328:	0d400693          	li	a3,212
    8000132c:	00001617          	auipc	a2,0x1
    80001330:	a1c60613          	add	a2,a2,-1508 # 80001d48 <etext+0x57c>
    80001334:	00001597          	auipc	a1,0x1
    80001338:	b0c58593          	add	a1,a1,-1268 # 80001e40 <etext+0x674>
    8000133c:	00001517          	auipc	a0,0x1
    80001340:	a2450513          	add	a0,a0,-1500 # 80001d60 <etext+0x594>
    80001344:	900ff0ef          	jal	80000444 <printf>
    80001348:	0001                	nop
    8000134a:	bffd                	j	80001348 <test_kernel_mapping+0x1b0>
    8000134c:	fe843783          	ld	a5,-24(s0)
    80001350:	639c                	ld	a5,0(a5)
    80001352:	8b91                	and	a5,a5,4
    80001354:	e39d                	bnez	a5,8000137a <test_kernel_mapping+0x1e2>
    80001356:	0d500693          	li	a3,213
    8000135a:	00001617          	auipc	a2,0x1
    8000135e:	9ee60613          	add	a2,a2,-1554 # 80001d48 <etext+0x57c>
    80001362:	00001597          	auipc	a1,0x1
    80001366:	e0658593          	add	a1,a1,-506 # 80002168 <etext+0x99c>
    8000136a:	00001517          	auipc	a0,0x1
    8000136e:	9f650513          	add	a0,a0,-1546 # 80001d60 <etext+0x594>
    80001372:	8d2ff0ef          	jal	80000444 <printf>
    80001376:	0001                	nop
    80001378:	bffd                	j	80001376 <test_kernel_mapping+0x1de>
    8000137a:	fe843783          	ld	a5,-24(s0)
    8000137e:	639c                	ld	a5,0(a5)
    80001380:	83a9                	srl	a5,a5,0xa
    80001382:	07b2                	sll	a5,a5,0xc
    80001384:	fef43023          	sd	a5,-32(s0)
    80001388:	fe043703          	ld	a4,-32(s0)
    8000138c:	fd843783          	ld	a5,-40(s0)
    80001390:	02f70463          	beq	a4,a5,800013b8 <test_kernel_mapping+0x220>
    80001394:	0d700693          	li	a3,215
    80001398:	00001617          	auipc	a2,0x1
    8000139c:	9b060613          	add	a2,a2,-1616 # 80001d48 <etext+0x57c>
    800013a0:	00001597          	auipc	a1,0x1
    800013a4:	dd858593          	add	a1,a1,-552 # 80002178 <etext+0x9ac>
    800013a8:	00001517          	auipc	a0,0x1
    800013ac:	9b850513          	add	a0,a0,-1608 # 80001d60 <etext+0x594>
    800013b0:	894ff0ef          	jal	80000444 <printf>
    800013b4:	0001                	nop
    800013b6:	bffd                	j	800013b4 <test_kernel_mapping+0x21c>
    800013b8:	00001517          	auipc	a0,0x1
    800013bc:	dd050513          	add	a0,a0,-560 # 80002188 <etext+0x9bc>
    800013c0:	884ff0ef          	jal	80000444 <printf>
    800013c4:	00001517          	auipc	a0,0x1
    800013c8:	de450513          	add	a0,a0,-540 # 800021a8 <etext+0x9dc>
    800013cc:	878ff0ef          	jal	80000444 <printf>
    800013d0:	00007797          	auipc	a5,0x7
    800013d4:	f3878793          	add	a5,a5,-200 # 80008308 <kernel_pagetable>
    800013d8:	639c                	ld	a5,0(a5)
    800013da:	4601                	li	a2,0
    800013dc:	100005b7          	lui	a1,0x10000
    800013e0:	853e                	mv	a0,a5
    800013e2:	e4aff0ef          	jal	80000a2c <walk>
    800013e6:	fea43423          	sd	a0,-24(s0)
    800013ea:	fe843783          	ld	a5,-24(s0)
    800013ee:	c791                	beqz	a5,800013fa <test_kernel_mapping+0x262>
    800013f0:	fe843783          	ld	a5,-24(s0)
    800013f4:	639c                	ld	a5,0(a5)
    800013f6:	8b85                	and	a5,a5,1
    800013f8:	e39d                	bnez	a5,8000141e <test_kernel_mapping+0x286>
    800013fa:	0dd00693          	li	a3,221
    800013fe:	00001617          	auipc	a2,0x1
    80001402:	94a60613          	add	a2,a2,-1718 # 80001d48 <etext+0x57c>
    80001406:	00001597          	auipc	a1,0x1
    8000140a:	bf258593          	add	a1,a1,-1038 # 80001ff8 <etext+0x82c>
    8000140e:	00001517          	auipc	a0,0x1
    80001412:	95250513          	add	a0,a0,-1710 # 80001d60 <etext+0x594>
    80001416:	82eff0ef          	jal	80000444 <printf>
    8000141a:	0001                	nop
    8000141c:	bffd                	j	8000141a <test_kernel_mapping+0x282>
    8000141e:	fe843783          	ld	a5,-24(s0)
    80001422:	639c                	ld	a5,0(a5)
    80001424:	8b91                	and	a5,a5,4
    80001426:	e39d                	bnez	a5,8000144c <test_kernel_mapping+0x2b4>
    80001428:	0de00693          	li	a3,222
    8000142c:	00001617          	auipc	a2,0x1
    80001430:	91c60613          	add	a2,a2,-1764 # 80001d48 <etext+0x57c>
    80001434:	00001597          	auipc	a1,0x1
    80001438:	d3458593          	add	a1,a1,-716 # 80002168 <etext+0x99c>
    8000143c:	00001517          	auipc	a0,0x1
    80001440:	92450513          	add	a0,a0,-1756 # 80001d60 <etext+0x594>
    80001444:	800ff0ef          	jal	80000444 <printf>
    80001448:	0001                	nop
    8000144a:	bffd                	j	80001448 <test_kernel_mapping+0x2b0>
    8000144c:	fe843783          	ld	a5,-24(s0)
    80001450:	639c                	ld	a5,0(a5)
    80001452:	83a9                	srl	a5,a5,0xa
    80001454:	07b2                	sll	a5,a5,0xc
    80001456:	fef43023          	sd	a5,-32(s0)
    8000145a:	fe043703          	ld	a4,-32(s0)
    8000145e:	100007b7          	lui	a5,0x10000
    80001462:	02f70463          	beq	a4,a5,8000148a <test_kernel_mapping+0x2f2>
    80001466:	0e000693          	li	a3,224
    8000146a:	00001617          	auipc	a2,0x1
    8000146e:	8de60613          	add	a2,a2,-1826 # 80001d48 <etext+0x57c>
    80001472:	00001597          	auipc	a1,0x1
    80001476:	d5658593          	add	a1,a1,-682 # 800021c8 <etext+0x9fc>
    8000147a:	00001517          	auipc	a0,0x1
    8000147e:	8e650513          	add	a0,a0,-1818 # 80001d60 <etext+0x594>
    80001482:	fc3fe0ef          	jal	80000444 <printf>
    80001486:	0001                	nop
    80001488:	bffd                	j	80001486 <test_kernel_mapping+0x2ee>
    8000148a:	00001517          	auipc	a0,0x1
    8000148e:	d4e50513          	add	a0,a0,-690 # 800021d8 <etext+0xa0c>
    80001492:	fb3fe0ef          	jal	80000444 <printf>
    80001496:	00001517          	auipc	a0,0x1
    8000149a:	d5a50513          	add	a0,a0,-678 # 800021f0 <etext+0xa24>
    8000149e:	fa7fe0ef          	jal	80000444 <printf>
    800014a2:	0001                	nop
    800014a4:	70a2                	ld	ra,40(sp)
    800014a6:	7402                	ld	s0,32(sp)
    800014a8:	6145                	add	sp,sp,48
    800014aa:	8082                	ret

00000000800014ac <w_stvec>:
    800014ac:	7179                	add	sp,sp,-48
    800014ae:	f422                	sd	s0,40(sp)
    800014b0:	1800                	add	s0,sp,48
    800014b2:	fca43c23          	sd	a0,-40(s0)
    800014b6:	fd843783          	ld	a5,-40(s0)
    800014ba:	fef43423          	sd	a5,-24(s0)
    800014be:	fe843783          	ld	a5,-24(s0)
    800014c2:	10579073          	csrw	stvec,a5
    800014c6:	0001                	nop
    800014c8:	7422                	ld	s0,40(sp)
    800014ca:	6145                	add	sp,sp,48
    800014cc:	8082                	ret

00000000800014ce <r_sstatus>:
    800014ce:	1101                	add	sp,sp,-32
    800014d0:	ec22                	sd	s0,24(sp)
    800014d2:	1000                	add	s0,sp,32
    800014d4:	100027f3          	csrr	a5,sstatus
    800014d8:	fef43423          	sd	a5,-24(s0)
    800014dc:	fe843783          	ld	a5,-24(s0)
    800014e0:	853e                	mv	a0,a5
    800014e2:	6462                	ld	s0,24(sp)
    800014e4:	6105                	add	sp,sp,32
    800014e6:	8082                	ret

00000000800014e8 <w_sstatus>:
    800014e8:	7179                	add	sp,sp,-48
    800014ea:	f422                	sd	s0,40(sp)
    800014ec:	1800                	add	s0,sp,48
    800014ee:	fca43c23          	sd	a0,-40(s0)
    800014f2:	fd843783          	ld	a5,-40(s0)
    800014f6:	fef43423          	sd	a5,-24(s0)
    800014fa:	fe843783          	ld	a5,-24(s0)
    800014fe:	10079073          	csrw	sstatus,a5
    80001502:	0001                	nop
    80001504:	7422                	ld	s0,40(sp)
    80001506:	6145                	add	sp,sp,48
    80001508:	8082                	ret

000000008000150a <r_scause>:
    8000150a:	1101                	add	sp,sp,-32
    8000150c:	ec22                	sd	s0,24(sp)
    8000150e:	1000                	add	s0,sp,32
    80001510:	142027f3          	csrr	a5,scause
    80001514:	fef43423          	sd	a5,-24(s0)
    80001518:	fe843783          	ld	a5,-24(s0)
    8000151c:	853e                	mv	a0,a5
    8000151e:	6462                	ld	s0,24(sp)
    80001520:	6105                	add	sp,sp,32
    80001522:	8082                	ret

0000000080001524 <r_sepc>:
    80001524:	1101                	add	sp,sp,-32
    80001526:	ec22                	sd	s0,24(sp)
    80001528:	1000                	add	s0,sp,32
    8000152a:	141027f3          	csrr	a5,sepc
    8000152e:	fef43423          	sd	a5,-24(s0)
    80001532:	fe843783          	ld	a5,-24(s0)
    80001536:	853e                	mv	a0,a5
    80001538:	6462                	ld	s0,24(sp)
    8000153a:	6105                	add	sp,sp,32
    8000153c:	8082                	ret

000000008000153e <w_sie>:
    8000153e:	7179                	add	sp,sp,-48
    80001540:	f422                	sd	s0,40(sp)
    80001542:	1800                	add	s0,sp,48
    80001544:	fca43c23          	sd	a0,-40(s0)
    80001548:	fd843783          	ld	a5,-40(s0)
    8000154c:	fef43423          	sd	a5,-24(s0)
    80001550:	fe843783          	ld	a5,-24(s0)
    80001554:	10479073          	csrw	sie,a5
    80001558:	0001                	nop
    8000155a:	7422                	ld	s0,40(sp)
    8000155c:	6145                	add	sp,sp,48
    8000155e:	8082                	ret

0000000080001560 <r_sie>:
    80001560:	1101                	add	sp,sp,-32
    80001562:	ec22                	sd	s0,24(sp)
    80001564:	1000                	add	s0,sp,32
    80001566:	104027f3          	csrr	a5,sie
    8000156a:	fef43423          	sd	a5,-24(s0)
    8000156e:	fe843783          	ld	a5,-24(s0)
    80001572:	853e                	mv	a0,a5
    80001574:	6462                	ld	s0,24(sp)
    80001576:	6105                	add	sp,sp,32
    80001578:	8082                	ret

000000008000157a <r_time>:
    8000157a:	1101                	add	sp,sp,-32
    8000157c:	ec22                	sd	s0,24(sp)
    8000157e:	1000                	add	s0,sp,32
    80001580:	c01027f3          	rdtime	a5
    80001584:	fef43423          	sd	a5,-24(s0)
    80001588:	fe843783          	ld	a5,-24(s0)
    8000158c:	853e                	mv	a0,a5
    8000158e:	6462                	ld	s0,24(sp)
    80001590:	6105                	add	sp,sp,32
    80001592:	8082                	ret

0000000080001594 <r_stval>:
    80001594:	1101                	add	sp,sp,-32
    80001596:	ec22                	sd	s0,24(sp)
    80001598:	1000                	add	s0,sp,32
    8000159a:	143027f3          	csrr	a5,stval
    8000159e:	fef43423          	sd	a5,-24(s0)
    800015a2:	fe843783          	ld	a5,-24(s0)
    800015a6:	853e                	mv	a0,a5
    800015a8:	6462                	ld	s0,24(sp)
    800015aa:	6105                	add	sp,sp,32
    800015ac:	8082                	ret

00000000800015ae <set_next_timer_interrupt>:
    800015ae:	1141                	add	sp,sp,-16
    800015b0:	e406                	sd	ra,8(sp)
    800015b2:	e022                	sd	s0,0(sp)
    800015b4:	0800                	add	s0,sp,16
    800015b6:	fc5ff0ef          	jal	8000157a <r_time>
    800015ba:	872a                	mv	a4,a0
    800015bc:	000f47b7          	lui	a5,0xf4
    800015c0:	24078793          	add	a5,a5,576 # f4240 <_entry-0x7ff0bdc0>
    800015c4:	97ba                	add	a5,a5,a4
    800015c6:	4681                	li	a3,0
    800015c8:	4601                	li	a2,0
    800015ca:	85be                	mv	a1,a5
    800015cc:	4501                	li	a0,0
    800015ce:	1b8000ef          	jal	80001786 <sbi_call>
    800015d2:	0001                	nop
    800015d4:	60a2                	ld	ra,8(sp)
    800015d6:	6402                	ld	s0,0(sp)
    800015d8:	0141                	add	sp,sp,16
    800015da:	8082                	ret

00000000800015dc <trap_init_s>:
    800015dc:	1141                	add	sp,sp,-16
    800015de:	e406                	sd	ra,8(sp)
    800015e0:	e022                	sd	s0,0(sp)
    800015e2:	0800                	add	s0,sp,16
    800015e4:	00000797          	auipc	a5,0x0
    800015e8:	11c78793          	add	a5,a5,284 # 80001700 <kernelvec>
    800015ec:	853e                	mv	a0,a5
    800015ee:	ebfff0ef          	jal	800014ac <w_stvec>
    800015f2:	f6fff0ef          	jal	80001560 <r_sie>
    800015f6:	87aa                	mv	a5,a0
    800015f8:	0207e793          	or	a5,a5,32
    800015fc:	853e                	mv	a0,a5
    800015fe:	f41ff0ef          	jal	8000153e <w_sie>
    80001602:	ecdff0ef          	jal	800014ce <r_sstatus>
    80001606:	87aa                	mv	a5,a0
    80001608:	0027e793          	or	a5,a5,2
    8000160c:	853e                	mv	a0,a5
    8000160e:	edbff0ef          	jal	800014e8 <w_sstatus>
    80001612:	f9dff0ef          	jal	800015ae <set_next_timer_interrupt>
    80001616:	00001517          	auipc	a0,0x1
    8000161a:	c1250513          	add	a0,a0,-1006 # 80002228 <etext+0xa5c>
    8000161e:	e27fe0ef          	jal	80000444 <printf>
    80001622:	0001                	nop
    80001624:	60a2                	ld	ra,8(sp)
    80001626:	6402                	ld	s0,0(sp)
    80001628:	0141                	add	sp,sp,16
    8000162a:	8082                	ret

000000008000162c <kerneltrap>:
    8000162c:	1101                	add	sp,sp,-32
    8000162e:	ec06                	sd	ra,24(sp)
    80001630:	e822                	sd	s0,16(sp)
    80001632:	1000                	add	s0,sp,32
    80001634:	ed7ff0ef          	jal	8000150a <r_scause>
    80001638:	fea43423          	sd	a0,-24(s0)
    8000163c:	fe843783          	ld	a5,-24(s0)
    80001640:	0607dc63          	bgez	a5,800016b8 <kerneltrap+0x8c>
    80001644:	fe843703          	ld	a4,-24(s0)
    80001648:	57fd                	li	a5,-1
    8000164a:	8385                	srl	a5,a5,0x1
    8000164c:	8ff9                	and	a5,a5,a4
    8000164e:	fef43023          	sd	a5,-32(s0)
    80001652:	fe043703          	ld	a4,-32(s0)
    80001656:	4795                	li	a5,5
    80001658:	04f71763          	bne	a4,a5,800016a6 <kerneltrap+0x7a>
    8000165c:	f53ff0ef          	jal	800015ae <set_next_timer_interrupt>
    80001660:	00007797          	auipc	a5,0x7
    80001664:	cb078793          	add	a5,a5,-848 # 80008310 <ticks>
    80001668:	639c                	ld	a5,0(a5)
    8000166a:	00178713          	add	a4,a5,1
    8000166e:	00007797          	auipc	a5,0x7
    80001672:	ca278793          	add	a5,a5,-862 # 80008310 <ticks>
    80001676:	e398                	sd	a4,0(a5)
    80001678:	00007797          	auipc	a5,0x7
    8000167c:	c9878793          	add	a5,a5,-872 # 80008310 <ticks>
    80001680:	6398                	ld	a4,0(a5)
    80001682:	06400793          	li	a5,100
    80001686:	02f777b3          	remu	a5,a4,a5
    8000168a:	e7ad                	bnez	a5,800016f4 <kerneltrap+0xc8>
    8000168c:	00007797          	auipc	a5,0x7
    80001690:	c8478793          	add	a5,a5,-892 # 80008310 <ticks>
    80001694:	639c                	ld	a5,0(a5)
    80001696:	85be                	mv	a1,a5
    80001698:	00001517          	auipc	a0,0x1
    8000169c:	bb850513          	add	a0,a0,-1096 # 80002250 <etext+0xa84>
    800016a0:	da5fe0ef          	jal	80000444 <printf>
    800016a4:	a881                	j	800016f4 <kerneltrap+0xc8>
    800016a6:	fe043583          	ld	a1,-32(s0)
    800016aa:	00001517          	auipc	a0,0x1
    800016ae:	bc650513          	add	a0,a0,-1082 # 80002270 <etext+0xaa4>
    800016b2:	d93fe0ef          	jal	80000444 <printf>
    800016b6:	a081                	j	800016f6 <kerneltrap+0xca>
    800016b8:	fe843583          	ld	a1,-24(s0)
    800016bc:	00001517          	auipc	a0,0x1
    800016c0:	bdc50513          	add	a0,a0,-1060 # 80002298 <etext+0xacc>
    800016c4:	d81fe0ef          	jal	80000444 <printf>
    800016c8:	e5dff0ef          	jal	80001524 <r_sepc>
    800016cc:	87aa                	mv	a5,a0
    800016ce:	85be                	mv	a1,a5
    800016d0:	00001517          	auipc	a0,0x1
    800016d4:	bf050513          	add	a0,a0,-1040 # 800022c0 <etext+0xaf4>
    800016d8:	d6dfe0ef          	jal	80000444 <printf>
    800016dc:	eb9ff0ef          	jal	80001594 <r_stval>
    800016e0:	87aa                	mv	a5,a0
    800016e2:	85be                	mv	a1,a5
    800016e4:	00001517          	auipc	a0,0x1
    800016e8:	bec50513          	add	a0,a0,-1044 # 800022d0 <etext+0xb04>
    800016ec:	d59fe0ef          	jal	80000444 <printf>
    800016f0:	0001                	nop
    800016f2:	bffd                	j	800016f0 <kerneltrap+0xc4>
    800016f4:	0001                	nop
    800016f6:	0001                	nop
    800016f8:	60e2                	ld	ra,24(sp)
    800016fa:	6442                	ld	s0,16(sp)
    800016fc:	6105                	add	sp,sp,32
    800016fe:	8082                	ret

0000000080001700 <kernelvec>:
    80001700:	716d                	add	sp,sp,-272
    80001702:	e006                	sd	ra,0(sp)
    80001704:	e40a                	sd	sp,8(sp)
    80001706:	e80e                	sd	gp,16(sp)
    80001708:	ec12                	sd	tp,24(sp)
    8000170a:	f016                	sd	t0,32(sp)
    8000170c:	f41a                	sd	t1,40(sp)
    8000170e:	f81e                	sd	t2,48(sp)
    80001710:	fc22                	sd	s0,56(sp)
    80001712:	e0a6                	sd	s1,64(sp)
    80001714:	e4aa                	sd	a0,72(sp)
    80001716:	e8ae                	sd	a1,80(sp)
    80001718:	ecb2                	sd	a2,88(sp)
    8000171a:	f0b6                	sd	a3,96(sp)
    8000171c:	f4ba                	sd	a4,104(sp)
    8000171e:	f8be                	sd	a5,112(sp)
    80001720:	fcc2                	sd	a6,120(sp)
    80001722:	e146                	sd	a7,128(sp)
    80001724:	e54a                	sd	s2,136(sp)
    80001726:	e94e                	sd	s3,144(sp)
    80001728:	ed52                	sd	s4,152(sp)
    8000172a:	f156                	sd	s5,160(sp)
    8000172c:	f55a                	sd	s6,168(sp)
    8000172e:	f95e                	sd	s7,176(sp)
    80001730:	fd62                	sd	s8,184(sp)
    80001732:	e1e6                	sd	s9,192(sp)
    80001734:	e5ea                	sd	s10,200(sp)
    80001736:	e9ee                	sd	s11,208(sp)
    80001738:	edf2                	sd	t3,216(sp)
    8000173a:	f1f6                	sd	t4,224(sp)
    8000173c:	f5fa                	sd	t5,232(sp)
    8000173e:	f9fe                	sd	t6,240(sp)
    80001740:	eedff0ef          	jal	8000162c <kerneltrap>
    80001744:	6082                	ld	ra,0(sp)
    80001746:	61c2                	ld	gp,16(sp)
    80001748:	6262                	ld	tp,24(sp)
    8000174a:	7282                	ld	t0,32(sp)
    8000174c:	7322                	ld	t1,40(sp)
    8000174e:	73c2                	ld	t2,48(sp)
    80001750:	7462                	ld	s0,56(sp)
    80001752:	6486                	ld	s1,64(sp)
    80001754:	6526                	ld	a0,72(sp)
    80001756:	65c6                	ld	a1,80(sp)
    80001758:	6666                	ld	a2,88(sp)
    8000175a:	7686                	ld	a3,96(sp)
    8000175c:	7726                	ld	a4,104(sp)
    8000175e:	77c6                	ld	a5,112(sp)
    80001760:	7866                	ld	a6,120(sp)
    80001762:	688a                	ld	a7,128(sp)
    80001764:	692a                	ld	s2,136(sp)
    80001766:	69ca                	ld	s3,144(sp)
    80001768:	6a6a                	ld	s4,152(sp)
    8000176a:	7a8a                	ld	s5,160(sp)
    8000176c:	7b2a                	ld	s6,168(sp)
    8000176e:	7bca                	ld	s7,176(sp)
    80001770:	7c6a                	ld	s8,184(sp)
    80001772:	6c8e                	ld	s9,192(sp)
    80001774:	6d2e                	ld	s10,200(sp)
    80001776:	6dce                	ld	s11,208(sp)
    80001778:	6e6e                	ld	t3,216(sp)
    8000177a:	7e8e                	ld	t4,224(sp)
    8000177c:	7f2e                	ld	t5,232(sp)
    8000177e:	7fce                	ld	t6,240(sp)
    80001780:	6151                	add	sp,sp,272
    80001782:	10200073          	sret

0000000080001786 <sbi_call>:
    80001786:	7139                	add	sp,sp,-64
    80001788:	fc22                	sd	s0,56(sp)
    8000178a:	0080                	add	s0,sp,64
    8000178c:	fca43c23          	sd	a0,-40(s0)
    80001790:	fcb43823          	sd	a1,-48(s0)
    80001794:	fcc43423          	sd	a2,-56(s0)
    80001798:	fcd43023          	sd	a3,-64(s0)
    8000179c:	fd843783          	ld	a5,-40(s0)
    800017a0:	fd043703          	ld	a4,-48(s0)
    800017a4:	fc843683          	ld	a3,-56(s0)
    800017a8:	fc043803          	ld	a6,-64(s0)
    800017ac:	88be                	mv	a7,a5
    800017ae:	853a                	mv	a0,a4
    800017b0:	85b6                	mv	a1,a3
    800017b2:	8642                	mv	a2,a6
    800017b4:	00000073          	ecall
    800017b8:	87aa                	mv	a5,a0
    800017ba:	fef43423          	sd	a5,-24(s0)
    800017be:	fe843783          	ld	a5,-24(s0)
    800017c2:	853e                	mv	a0,a5
    800017c4:	7462                	ld	s0,56(sp)
    800017c6:	6121                	add	sp,sp,64
    800017c8:	8082                	ret
	...
