FC = gfortran
FFLAGS = -O2 -Wall -Wno-unused-dummy-argument
SRCDIR = src
DEMODIR = demo
BINDIR = bin

.PHONY: all clean demo

all: $(BINDIR)/primos_valencia.exe $(BINDIR)/crivo_segmentado.exe

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/primos_mod.o: $(SRCDIR)/primos_mod.f90 | $(BINDIR)
	$(FC) $(FFLAGS) -c $< -o $@

$(BINDIR)/primos_valencia.exe: $(SRCDIR)/primos_valencia.f90 $(BINDIR)/primos_mod.o | $(BINDIR)
	$(FC) $(FFLAGS) $< $(BINDIR)/primos_mod.o -o $@

$(BINDIR)/crivo_segmentado.exe: $(SRCDIR)/crivo_segmentado.f90 | $(BINDIR)
	$(FC) $(FFLAGS) $< -o $@

all: $(BINDIR)/miller_rabin_128.o

$(BINDIR)/miller_rabin_128.o: $(SRCDIR)/miller_rabin_128.f90 | $(BINDIR)
	$(FC) $(FFLAGS) -c $< -o $@

DEMOS = verifica_gap.exe busca_gap_rapido.exe maior_primo.exe verifica_gap_grande.exe test_mr128.exe

demo: $(addprefix $(BINDIR)/, $(DEMOS))

$(BINDIR)/verifica_gap.exe: $(DEMODIR)/verifica_gap.f90 $(BINDIR)/primos_mod.o | $(BINDIR)
	$(FC) $(FFLAGS) $< $(BINDIR)/primos_mod.o -o $@

$(BINDIR)/busca_gap_rapido.exe: $(DEMODIR)/busca_gap_rapido.f90 $(BINDIR)/primos_mod.o | $(BINDIR)
	$(FC) $(FFLAGS) $< $(BINDIR)/primos_mod.o -o $@

$(BINDIR)/maior_primo.exe: $(DEMODIR)/maior_primo.f90 $(BINDIR)/primos_mod.o | $(BINDIR)
	$(FC) $(FFLAGS) $< $(BINDIR)/primos_mod.o -o $@

$(BINDIR)/verifica_gap_grande.exe: $(DEMODIR)/verifica_gap_grande.f90 $(BINDIR)/primos_mod.o | $(BINDIR)
	$(FC) $(FFLAGS) $< $(BINDIR)/primos_mod.o -o $@

$(BINDIR)/test_mr128.exe: $(DEMODIR)/test_mr128.f90 $(BINDIR)/miller_rabin_128.o | $(BINDIR)
	$(FC) $(FFLAGS) -fno-range-check $< $(BINDIR)/miller_rabin_128.o -o $@

clean:
	rm -rf $(BINDIR) *.mod
