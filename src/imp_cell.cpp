#include "imp_cell.h"

using namespace std;

void ImpCell::Clear() {
 for (auto ptr: dep_ptrs)
   ptr->Invalidate();

 if (formula)
   for (auto ptr: formula->GetRefPtrs())
     ptr->RemoveDepPtr(this);

 Invalidate();
 formula.reset(nullptr);
 raw_text.clear();
 rendered_text.clear();
 dep_ptrs.clear();
}
