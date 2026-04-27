SRCS=./FVM.tex
OUT=FVM

all: ${SRCS}
	pdflatex -interaction=nonstopmode $^ >/dev/null
	@echo "Compilation finished succesfully"

verbose: ${SRCS}
	pdflatex -interaction=nonstopmode $^

clean:
	rm -rf ./*.aux ./${OUT}.pdf

