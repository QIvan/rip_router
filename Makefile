# Каталоги
IDIR = include
CDIR = src
ODIR = bin
TEXDIR = tex
TEXINCDIR = $(TEXDIR)/include
DOXDIR = doxygen
UDIR = utils
DOTDIR = dot

# Утилитки
MAKE2DOT = $(UDIR)/makefile2dot
MAKESIMPLE = $(UDIR)/makesimple
RE2TEX = $(UDIR)/re2tex
FSM2DOT = $(UDIR)/fsm2dot
CFLOW=cflow --level "0= "
CFLOW2DOT=$(UDIR)/cflow2dot
# Файл cflow.ignore содержит список функций,
# исключаемых из графов вызовов.
SIMPLECFLOW=grep -v -f cflow.ignore
SRC2TEX=$(UDIR)/src2tex

# Главная программа
PROG = rip_router

# Отчёт
REPORT = report.pdf

# Компилятор С
CC = gcc
# Флаги компиляции
CFLAGS = -I$(IDIR) -Wall -std=c99
# -Werror
# Флаги сборки
LDFLAGS += $(shell autoopts-config ldflags)

# latex -> pdf
PDFLATEX = pdflatex -interaction=nonstopmode

# Файл с регулярными выражениями
RE_FILE = $(IDIR)/server-re.h
# Файл с графом процесса сборки программы
Makefile_1.dot = $(DOTDIR)/Makefile_1.dot

# По-умолчанию и по make all собираем и отчёт и программу.
# Отдельно отчёт собирается как make report
all: $(PROG) # $(REPORT)

# Добавляются имена сгенерённых файлов.
# GENINCLUDES = $(IDIR)/checkoptn.h $(IDIR)/server-fsm.h

INCLUDES = $(wildcard $(IDIR)/*.h) $(IDIR)/checkoptn.h $(IDIR)/server-fsm.h
# $(wildcard $(CDIR)/*.c)
CSRC = $(addprefix src/, main.c) # $(addprefix src/, checkoptn.c server-fsm.c server.c server-cmd.c server-parse.c server-run.c server-state.c)

# Объектные файлы. Обычно, наоборот, по заданному списку объектных получают
# список исходных файлов. ЕНо мне лень.
OBJS = $(patsubst $(CDIR)/%, $(ODIR)/%, $(patsubst %.c,%.o,$(CSRC)))

# Файлы latex
TEXS = $(wildcard $(TEXDIR)/*.tex)

# Нафиг эту фигню, getopt'а вполне хватит.
# Кодогенерация: разбор параметров командной строки
#$(CDIR)/checkoptn.c: $(CDIR)/checkoptn.def
#	cd $(CDIR) && SHELL=/bin/sh autogen checkoptn.def
#$(IDIR)/checkoptn.h:  $(CDIR)/checkoptn.def
#	cd $(CDIR) && SHELL=/bin/sh autogen checkoptn.def
#	mv -f $(CDIR)/checkoptn.h $(IDIR)

# Кодогенерация: конечный автомат
$(CDIR)/server-fsm.c: $(CDIR)/server.def
	cd $(CDIR) && autogen server.def
$(IDIR)/server-fsm.h: $(CDIR)/server.def
	cd $(CDIR) && autogen server.def
	mv -f $(CDIR)/server-fsm.h $(IDIR)

# Сгенерённый код компилируется особым образом
# (как того требует инструкция).
$(ODIR)/checkoptn.o: $(CDIR)/checkoptn.c
	$(CC) -c -I$(IDIR) -DTEST_CHECK_OPTS `autoopts-config cflags` -o $@  $<

$(DOTDIR)/%_def.dot: $(CDIR)/%.def
	$(FSM2DOT) $< > $@

# .dot -> _dot.tex
$(TEXINCDIR)/%_dot.tex: $(DOTDIR)/%.dot
	dot2tex -ftikz --autosize --crop  $< > $@

# _dot.tex -> _dot.pdf
$(TEXINCDIR)/%_dot.pdf: $(TEXINCDIR)/%_dot.tex
	$(PDFLATEX) -output-directory $(TEXINCDIR) $<

$(TEXINCDIR)/%_re.tex: $(RE_FILE)
	$(RE2TEX) $(RE_FILE) $(TEXINCDIR)

$(ODIR)/%.o: $(CDIR)/%.c $(INCLUDES) # $(GENINCLUDES)
	$(CC) -c $(CFLAGS) -o $@ $<

$(PROG): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^



# Тестирование
.PHONY: tests
tests: test_units test_memory test_style test_system

.PHONY: test_units
test_units:
	echo Сделайте сами на основе check.sourceforge.net

.PHONY: test_style
test_style:
	echo Сделайте сами на основе astyle.sourceforge.net или checkpatch.pl
	echo Для инженеров -- не обязательно

.PHONY: test_memory
test_memory: $(PROG)
	echo Сделайте сами на основе valgrind

.PHONY: test_system
test_system: $(PROG)
	echo Сделайте сами на основе тестирования через netcat и/или скриптов на python/perl/etc.
	./$(PROG) -p 1025 -f tests/test01.cmds



.PHONY: doxygen
doxygen: doxygen.cfg $(CSRC) $(INCLUDES)
	doxygen doxygen.cfg
	mkdir -p $(TEXINCDIR)
	cp $(DOXDIR)/latex/*.tex $(TEXINCDIR)
	cp $(DOXDIR)/latex/*.sty $(TEXDIR)

.PHONY: clean
clean:
	rm -rf $(ODIR)/*.o $(CDIR)/*~ $(IDIR)/*~ $(TEXINCDIR)/* $(DOXDIR)/* $(DOTDIR)/*.dot; \
	rm $(PROG) \
	#rm simplest.mk *~; \
	find $(TEXDIR)/ -type f ! -name "*.tex" -exec rm -f {} \;

