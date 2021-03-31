POLYXB := ../polyxb
POLYXB_FLAGS := -p -t -M 4 -N 8 -K 6

BM_DIR := ../benchmarks

TESTS := mv mv-init \
	mm mm-init mm-act mm-init-act bmm\
	conv conv-init conv-act conv-init-act pooling \
	mlp3 conv2

RESULTS := $(addsuffix .txt,$(TESTS))

ANS := $(addprefix ans/,$(addsuffix .ans,$(TESTS)))

all: ans test

test: $(RESULTS)

%.txt: $(BM_DIR)/%.c
	$(POLYXB) $(POLYXB_FLAGS) -o $(basename $@).polyxb.c $^
	gcc -DTEST -g -o $(basename $@).polyxb $(basename $@).polyxb.c test.c -lm
	./$(basename $@).polyxb > $@ 2>&1

ans: $(ANS)

ans/%.ans: $(BM_DIR)/%.c
	mkdir -p ans
	gcc -g -o $(notdir $(basename $@)) $^ $(BM_DIR)/common.c -lm
	./$(notdir $(basename $@)) > $@ 2>&1

clean:
	rm -rf *.polyxb *.polyxb.c $(TESTS) *.txt ans

testclean:
	rm -rf *.polyxb *.polyxb.c *.txt

.PHONY: all clean test ans testclean
