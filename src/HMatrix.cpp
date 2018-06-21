#include "HMatrix.h"
#include "HConst.h"
#include <cmath>
#include <cassert>
//For printf
#include <cstdio>

void HMatrix::setup(int numCol_, int numRow_, const int *Astart_,
		    const int *Aindex_, const double *Avalue_, const int *nonbasicFlag_) {
  //Copy the A matrix and setup row-wise matrix with the nonbasic
  //columns before the basic columns for a general set of nonbasic
  //variables
  //
  //Copy A
  numCol = numCol_;
  numRow = numRow_;
  Astart.assign(Astart_, Astart_ + numCol_ + 1);
  
  int AcountX = Astart_[numCol_];
  Aindex.assign(Aindex_, Aindex_ + AcountX);
  Avalue.assign(Avalue_, Avalue_ + AcountX);

  // Build row copy - pointers
  vector<int> AR_Bend;
  ARstart.resize(numRow + 1);
  AR_Nend.assign(numRow, 0);
  AR_Bend.assign(numRow, 0);
  //Count the nonzeros of nonbasic and basic columns in each row
  for (int iCol = 0; iCol < numCol; iCol++) {
    if (nonbasicFlag_[iCol]) {
      for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
	int iRow = Aindex[k];
	AR_Nend[iRow]++;
      }
    } else {
      for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
	int iRow = Aindex[k];
	AR_Bend[iRow]++;
      }
    }
  }
  ARstart[0] = 0;
  for (int i = 0; i < numRow; i++)
    ARstart[i+1] = ARstart[i] + AR_Nend[i] + AR_Bend[i];
  for (int i = 0; i < numRow; i++) {
    AR_Bend[i] = ARstart[i] + AR_Nend[i];
    AR_Nend[i] = ARstart[i];
  }
  // Build row copy - elements
  ARindex.resize(AcountX);
  ARvalue.resize(AcountX);
  for (int iCol = 0; iCol < numCol; iCol++) {
    if (nonbasicFlag_[iCol]) {
      for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
	int iRow = Aindex[k];
	int iPut = AR_Nend[iRow]++;
	ARindex[iPut] = iCol;
	ARvalue[iPut] = Avalue[k];
      }
    } else {
      for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
	int iRow = Aindex[k];
	int iPut = AR_Bend[iRow]++;
	ARindex[iPut] = iCol;
	ARvalue[iPut] = Avalue[k];
      }
    }
  }
  //Initialise the density of the Price result
  //  row_apDensity = 0;
#ifdef JAJH_dev
  assert(setup_ok(nonbasicFlag_));
#endif
}

bool HMatrix::setup_ok(const int *nonbasicFlag_) {
  printf("Checking row-wise matrix\n");
  for (int row = 0; row < numRow; row++) {
    for (int el = ARstart[row]; el < AR_Nend[row]; el++) {
      int col = ARindex[el];
      if (!nonbasicFlag_[col]) {
	printf("Row-wise matrix error: col %d, (el = %d for row %d) is basic\n", col, el, row);
	return false;
      }
    }
    for (int el = AR_Nend[row]; el < ARstart[row+1]; el++) {
      int col = ARindex[el];
      if (nonbasicFlag_[col]) {
	printf("Row-wise matrix error: col %d, (el = %d for row %d) is nonbasic\n", col, el, row);
	return false;
      }
    }
  }
  return true;
}

void HMatrix::setup_lgBs(int numCol_, int numRow_, const int *Astart_,
			 const int *Aindex_, const double *Avalue_) {
  //Copy the A matrix and setup row-wise matrix with the nonbasic
  //columns before the basic columns for a logical basis
  //
  //Copy A
  numCol = numCol_;
  numRow = numRow_;
  Astart.assign(Astart_, Astart_ + numCol_ + 1);
  
  int AcountX = Astart_[numCol_];
  Aindex.assign(Aindex_, Aindex_ + AcountX);
  Avalue.assign(Avalue_, Avalue_ + AcountX);
  
  // Build row copy - pointers
  ARstart.resize(numRow + 1);
  AR_Nend.assign(numRow, 0);
  for (int k = 0; k < AcountX; k++)
    AR_Nend[Aindex[k]]++;
  ARstart[0] = 0;
  for (int i = 1; i <= numRow; i++)
    ARstart[i] = ARstart[i - 1] + AR_Nend[i - 1];
  for (int i = 0; i < numRow; i++)
    AR_Nend[i] = ARstart[i];
  
  // Build row copy - elements
  ARindex.resize(AcountX);
  ARvalue.resize(AcountX);
  for (int iCol = 0; iCol < numCol; iCol++) {
    for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
      int iRow = Aindex[k];
      int iPut = AR_Nend[iRow]++;
      ARindex[iPut] = iCol;
      ARvalue[iPut] = Avalue[k];
    }
  }
  //Initialise the density of the Price result
  //  row_apDensity = 0;
}

void HMatrix::update(int columnIn, int columnOut) {
    if (columnIn < numCol) {
        for (int k = Astart[columnIn]; k < Astart[columnIn + 1]; k++) {
            int iRow = Aindex[k];
            int iFind = ARstart[iRow];
            int iSwap = --AR_Nend[iRow];
            while (ARindex[iFind] != columnIn)
                iFind++;
            swap(ARindex[iFind], ARindex[iSwap]);
            swap(ARvalue[iFind], ARvalue[iSwap]);
        }
    }

    if (columnOut < numCol) {
        for (int k = Astart[columnOut]; k < Astart[columnOut + 1]; k++) {
            int iRow = Aindex[k];
            int iFind = AR_Nend[iRow];
            int iSwap = AR_Nend[iRow]++;
            while (ARindex[iFind] != columnOut)
                iFind++;
            swap(ARindex[iFind], ARindex[iSwap]);
            swap(ARvalue[iFind], ARvalue[iSwap]);
        }
    }
	//rp_mtx();
}

double HMatrix::compute_dot(HVector& vector, int iCol) const {
    double result = 0;
    if (iCol < numCol) {
        for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++)
            result += vector.array[Aindex[k]] * Avalue[k];
    } else {
        result = vector.array[iCol - numCol];
    }
    return result;
}

void HMatrix::collect_aj(HVector& vector, int iCol, double multi) const {
    if (iCol < numCol) {
        for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
            int index = Aindex[k];
            double value0 = vector.array[index];
            double value1 = value0 + multi * Avalue[k];
            if (value0 == 0)
                vector.index[vector.count++] = index;
            vector.array[index] =
                    (fabs(value1) < HSOL_CONST_TINY) ? HSOL_CONST_ZERO : value1;
        }
    } else {
        int index = iCol - numCol;
        double value0 = vector.array[index];
        double value1 = value0 + multi;
        if (value0 == 0)
            vector.index[vector.count++] = index;
        vector.array[index] =
                (fabs(value1) < HSOL_CONST_TINY) ? HSOL_CONST_ZERO : value1;
    }
}

void HMatrix::price_by_col(HVector& row_ap, HVector& row_ep) const {
    // Alias
    int ap_count = 0;
    int *ap_index = &row_ap.index[0];
    double *ap_array = &row_ap.array[0];
    const double *ep_array = &row_ep.array[0];

    // Computation
    for (int iCol = 0; iCol < numCol; iCol++) {
        double value = 0;
        for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
            value += ep_array[Aindex[k]] * Avalue[k];
        }
        if (fabs(value) > HSOL_CONST_TINY) {
            ap_array[iCol] = value;
            ap_index[ap_count++] = iCol;
        }
    }
    row_ap.count = ap_count;
    //    row_apDensity = (1-densityRunningAverageMu) * row_apDensity +
    //      densityRunningAverageMu * (1.0 * row_ap.count / numCol);

}

void HMatrix::price_by_row(HVector& row_ap, HVector& row_ep) const {
    // Alias
    int ap_count = 0;
    int *ap_index = &row_ap.index[0];
    double *ap_array = &row_ap.array[0];
    const int ep_count = row_ep.count;
    const int *ep_index = &row_ep.index[0];
    const double *ep_array = &row_ep.array[0];

    // Computation
    for (int i = 0; i < ep_count; i++) {
        int iRow = ep_index[i];
        double multi = ep_array[iRow];
        for (int k = ARstart[iRow]; k < AR_Nend[iRow]; k++) {
            int index = ARindex[k];
            double value0 = ap_array[index];
            double value1 = value0 + multi * ARvalue[k];
            if (value0 == 0)
                ap_index[ap_count++] = index;
	    //TODO Unlikely, but possible for ap_count to reach numCol
	    assert(ap_count<numCol);
            ap_array[index] =
                    (fabs(value1) < HSOL_CONST_TINY) ? HSOL_CONST_ZERO : value1;
        }
    }

    // Try to remove cancellation
    const int apcount1 = ap_count;
    ap_count = 0;
    for (int i = 0; i < apcount1; i++) {
        const int index = ap_index[i];
        const double value = ap_array[index];
        if (fabs(value) > HSOL_CONST_TINY) {
            ap_index[ap_count++] = index;
        } else {
            ap_array[index] = 0;
        }
    }
    row_ap.count = ap_count;
    //    row_apDensity = (1-densityRunningAverageMu) * row_apDensity +
    //      densityRunningAverageMu * (1.0 * row_ap.count / numCol);
}

void HMatrix::price_by_row_w_sw(HVector& row_ap, HVector& row_ep, double hist_dsty) const {
  // Alias
  int ap_count = 0;
  int *ap_index = &row_ap.index[0];
  double *ap_array = &row_ap.array[0];
  const int ep_count = row_ep.count;
  const int *ep_index = &row_ep.index[0];
  const double *ep_array = &row_ep.array[0];

  // Computation
  int nextI;
  //Determine whether to start hyper-sparse Price  
  nextI = 0;
  if (hist_dsty <= hyperPRICE) {
    for (int i = 0; i < ep_count; i++) {
      int iRow = ep_index[i];
      int iRowNNz = AR_Nend[iRow]-ARstart[iRow];
      bool price_by_row_sw = ap_count+iRowNNz >= numCol ||
	ap_count*price_by_row_sw_dsty_den > numCol*price_by_row_sw_dsty_num;
      //    if (price_by_row_sw) printf("Stop maintaining nonzeros in Price\n");
      if (price_by_row_sw) break;
      double multi = ep_array[iRow];
      for (int k = ARstart[iRow]; k < AR_Nend[iRow]; k++) {
	int index = ARindex[k];
	double value0 = ap_array[index];
	double value1 = value0 + multi * ARvalue[k];
	if (value0 == 0) ap_index[ap_count++] = index;
	ap_array[index] = (fabs(value1) < HSOL_CONST_TINY) ? HSOL_CONST_ZERO : value1;
      }
      nextI = i+1;
    }
  }
    
  if (nextI < ep_count) {
    //Price is not complete: finish without maintaining nonzeros of result
    for (int i = nextI; i < ep_count; i++) {
      int iRow = ep_index[i];
      double multi = ep_array[iRow];
      for (int k = ARstart[iRow]; k < AR_Nend[iRow]; k++) {
	int index = ARindex[k];
	double value0 = ap_array[index];
	double value1 = value0 + multi * ARvalue[k];
	//	ap_array[index] = value1;
	ap_array[index] = (fabs(value1) < HSOL_CONST_TINY) ? HSOL_CONST_ZERO : value1;
      }
    }
    //Determine indices of nonzeros in Price result
    int ap_count = 0;
    for (int index=0; index<numCol; index++) {
      double value1 = ap_array[index];
      if (fabs(value1) < HSOL_CONST_TINY) {
	ap_array[index] = 0;
      } else {
	ap_index[ap_count++] = index;
      }
    }
    row_ap.count = ap_count;
  }
  else {
    //Price is complete maintaining nonzeros of result
    // Try to remove cancellation
    const int apcount1 = ap_count;
    ap_count = 0;
    for (int i = 0; i < apcount1; i++) {
      const int index = ap_index[i];
      const double value = ap_array[index];
      if (fabs(value) > HSOL_CONST_TINY) {
	ap_index[ap_count++] = index;
      } else {
	ap_array[index] = 0;
      }
    }
    row_ap.count = ap_count;
  }
}

void HMatrix::price_er_ck(HVector& row_ap, HVector& row_ep) const {
  // Alias
  int *ap_index = &row_ap.index[0];
  double *ap_array = &row_ap.array[0];

  HVector lc_row_ap;
  lc_row_ap.setup(numCol);
  //  int *lc_ap_index = &lc_row_ap.index[0];
  double *lc_ap_array = &lc_row_ap.array[0];

  price_by_row(lc_row_ap, row_ep);

  double priceErTl=1e-4;
  double priceEr1=0;
  double row_apNormCk=0;
  double mxTinyVEr=0;
  for (int index=0; index<numCol; index++) {
    double PriceV = ap_array[index];
    double lcPriceV = lc_ap_array[index];
    if ((fabs(PriceV) > HSOL_CONST_TINY && fabs(lcPriceV) <= HSOL_CONST_TINY)
      || (fabs(lcPriceV) > HSOL_CONST_TINY && fabs(PriceV) <= HSOL_CONST_TINY))
      mxTinyVEr = max(max(fabs(PriceV), fabs(lcPriceV)),mxTinyVEr);
    //      printf("Index %7d: Small value inconsistency PriceV = %11.4g; lcPriceV = %11.4g\n", index, PriceV, lcPriceV);
    double dlPriceV = abs(PriceV - lcPriceV);
    priceEr1 += dlPriceV*dlPriceV;
    row_apNormCk += PriceV*PriceV;
    lc_ap_array[index]=0;
  }
  double row_apNorm=sqrt(row_apNormCk);

  bool row_apCountEr = lc_row_ap.count != row_ap.count;

  for (int i=0; i<row_ap.count; i++) {
    int index = ap_index[i];
    double PriceV = ap_array[index];
    row_apNormCk -= PriceV*PriceV;
    lc_ap_array[index]=PriceV;
    ap_array[index]=0;
  }
  double priceEr2=0;
  for (int index=0; index<numCol; index++) {
    double PriceV = ap_array[index];
    priceEr2 += PriceV*PriceV;
    ap_array[index] = lc_ap_array[index];
  }
  priceEr1 = sqrt(priceEr1);
  priceEr2 = sqrt(priceEr2);
  row_apNormCk = sqrt(abs(row_apNormCk));
  double row_apNormCkTl = 1e-7;
  bool row_apNormCkEr = row_apNormCk > row_apNormCkTl*row_apNorm;
  if (row_apCountEr
      ||priceEr1 > priceErTl
      || priceEr2 > priceErTl
      || row_apNormCkEr) {
    printf("Price error");
    if (priceEr1 > priceErTl) printf(": ||row_apDl|| = %11.4g", priceEr1);
    if (priceEr2 > priceErTl) printf(": ||row_apNZ|| = %11.4g", priceEr2);
    if (row_apNormCkEr) printf(": ||IxCk|| = %11.4g ||row_ap|| = %11.4g, Tl=%11.4g", row_apNormCk, row_apNorm, row_apNormCkTl*row_apNorm);
    if (row_apCountEr) printf(": row_apCountEr with mxTinyVEr = %11.4g", mxTinyVEr);
    printf("\n");
  }
}

void HMatrix::compute_vecT_matB(const double *vec, const int *base,
        HVector *result) {
    result->clear();
    int resultCount = 0;
    int *resultIndex = &result->index[0];
    double *resultArray = &result->array[0];
    for (int i = 0; i < numRow; i++) {
        int iCol = base[i];
        double value = 0;
        if (iCol < numCol) {
            for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++)
                value += vec[Aindex[k]] * Avalue[k];
        } else {
            value = vec[iCol - numCol];
        }
        if (fabs(value) > HSOL_CONST_TINY) {
            resultArray[i] = value;
            resultIndex[resultCount++] = i;
        }
    }
    result->count = resultCount;
}

void HMatrix::compute_matB_vec(const double *vec, const int *base,
        HVector *result) {
    result->clear();
    int resultCount = 0;
    int *resultIndex = &result->index[0];
    double *resultArray = &result->array[0];

    for (int i = 0; i < numRow; i++) {
        int iCol = base[i];
        double value = vec[i];
        if (fabs(value) > HSOL_CONST_TINY) {
            if (iCol < numCol) {
                for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++)
                    resultArray[Aindex[k]] += value * Avalue[k];
            } else {
                resultArray[iCol - numCol] += value;
            }
        }
    }

    for (int i = 0; i < numRow; i++) {
        if (fabs(resultArray[i]) > HSOL_CONST_TINY) {
            resultIndex[resultCount++] = i;
        } else {
            resultArray[i] = 0;
        }
    }
    result->count = resultCount;
}

void HMatrix::rp_mtx() {
	if (numRow+numCol>20) return;
    vector<double> rp_mtx_r;
    rp_mtx_r.assign(numCol, 0);

	printf("\nRow-wise matrix\n");
	printf("         ");
	for (int i = 0; i < numCol; i++) {
		printf(" %8d", i);
	}
	printf("\n");

    for (int i = 0; i < numRow; i++) {
		printf(" Row %2d: ", i);
        for (int k = ARstart[i]; k < AR_Nend[i]; k++) {
        	rp_mtx_r[ARindex[k]] = ARvalue[k];
        }
        for (int k = 0; k < numCol; k++) {
        	printf(" %8g", rp_mtx_r[k]);
        }
    	printf("\n");
        for (int k = ARstart[i]; k < AR_Nend[i]; k++) {
        	 rp_mtx_r[ARindex[k]] = 0;
        }
    }

}
