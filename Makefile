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

demo: $(BINDIR)/verifica_gap.exe

$(BINDIR)/verifica_gap.exe: $(DEMODIR)/verifica_gap.f90 $(BINDIR)/primos_mod.o | $(BINDIR)
	$(FC) $(FFLAGS) $< $(BINDIR)/primos_mod.o -o $@

clean:
	rm -rf $(BINDIR) *.mod
