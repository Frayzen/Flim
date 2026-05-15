#include <optional>
#include <tuple>
enum FVMAstType { NULL, PLUS, MINUS, UNKNOWN_IMPLICIT, UNKOWN_EXPLICIT };

class FVMAst {
public:
  template <typename T> FVMAst operator*(T f) { return FVMAst() }

private:
  FVMAst(FVMAstType type) : FVMAst(type, NULL, NULL) {};
  FVMAst(FVMAstType type, FVMAst leftChild) : FVMAst(type, leftChild, NULL) {};
  FVMAst(FVMAstType type, FVMAst leftChild, FVMAst rightChild)
      : type(type), leftChild(leftChild), rightChild(rightChild) {};
  FVMAstType type;
  std::optional<FVMAst> leftChild;
  FVMAst rightChild;
  friend FVMAst getImplicitUnkown(void);
  friend FVMAst getExplicitUnkown(void);
};

FVMAst getImplicitUnkown(void);
FVMAst getExplicitUnkown(void);
