#include <iostream>
#include <simPlPlace.h>
//#include <Eigen/Sparse>
#include <cgsolver.h>
#include <algorithm>
#include <vector>
#include <fstream>

//using namespace Eigen;
using namespace std;

bool greaterCol(const Triple& a, const Triple& b) {
	if (a.column == b.column) {
		return (a.row < b.row);
	} else {
		return (a.column < b.column);
	}
}

//void eigen(int n, vector<Triple> data, vector<double> bx, vector<double>& sol) {
//	VectorXd x(n), b(n);
//	SparseMatrix<double> A(n, n);
//	vector<Triplet<double> > dataT;
//	// fill A and b
//	for (int i = 0; i < data.size(); ++i) {
//		Triple t = data[i];
//		Triplet<double> tt(t.row, t.column, t.element);
//		dataT.push_back(tt);
//	}
//	for (int i = 0; i < bx.size(); ++i) {
//		b[i] = bx[i];
//	}
//
//	A.setFromTriplets(dataT.begin(), dataT.end());
//
//	ConjugateGradient<SparseMatrix<double> > cg;
//	cg.compute(A);
//	//	cg.setMaxIterations(500);
//	//
//	//	x = cg.solve(b);
//	//	std::cout << "#iterations:     " << cg.iterations() << std::endl;
//	//	std::cout << "estimated error: " << cg.error() << std::endl;
//	//	// update b, and solve again
//	//	x = cg.solve(b);
//
//	//	x = VectorXd::Random(n);
//	cg.setMaxIterations(1);
//	int iter = 0;
//	do {
//		x = cg.solveWithGuess(b, x);
//		cout << iter << " : " << cg.error() << std::endl;
//		++iter;
//	} while (cg.info() != Success && iter < 100);
//
//	sol.clear();
//	for (int i = 0; i < n; ++i) {
//		sol.push_back(x[i]);
//	}
//}

void dumpFile(vector<Triple> data, vector<double> bx, vector<double> sol,
		char* axFile, char* bxFile, char* solFile) {
	ofstream fout;
	fout.open(axFile);
	for (int i = 0; i < data.size(); ++i) {
		if (data[i].row == data[i].column) {
			fout << data[i].row + 1 << " " << data[i].column + 1 << " "
					<< data[i].element << endl;
		} else {
			fout << data[i].row + 1 << " " << data[i].column + 1 << " "
					<< data[i].element << endl;
			fout << data[i].column + 1 << " " << data[i].row + 1 << " "
					<< data[i].element << endl;
		}
	}
	fout.close();

	fout.open(bxFile);
	for (int i = 0; i < bx.size(); ++i) {
		fout << bx[i] << endl;
	}
	fout.close();

	fout.open(solFile);
	for (int i = 0; i < sol.size(); ++i) {
		fout << sol[i] << endl;
	}
	fout.close();
}

void cgSolver(int n, vector<Triple>& data, vector<double> bx,
		vector<double>& sol) {
	//cout<<"cgsolver: n = "<<n<<endl;
	//cout<<data[0].row<<" "<<data[0].column<<endl;
	QSparseMat M(n, n, data.size());

	sort(data.begin(), data.end(), greaterCol);

	for (int i = 0; i < data.size(); ++i) {
		assert((int)data[i].row < n);
		assert((int)data[i].column < n);
//		if (data[i].row == 30180){
//			cout<<data[i].row<<" "<<data[i].column<<" "<<data[i].element<<endl;
//					long x;
//					cin >> x;
//		}
		M.insertEnt((int) data[i].row, (int) data[i].column, data[i].element);
	}
	double* b = new double[n];
	double* x = new double[n];
	for (int i = 0; i < n; ++i) {
		b[i] = bx[i];
		x[i] = 0;
	}
	cgsolver(M, b, x, 500, 1e-6);

	sol.clear();
	for (int i = 0; i < n; ++i) {
		sol.push_back(x[i]);
	}
	delete[] b;
	delete[] x;

	//for debug
//	long count[11] = { 0 };
//	double maxValue = 0;
//	double minValue = 999;
//	long maxIndex = 0;
//	long minIndex = 0;
//	double value = 0;
//	for (int i = 0; i < data.size(); ++i) {
//		value = (abs(data[i].element));
//		if (value > maxValue){
//			maxValue = value;
//			maxIndex = data[i].row;
//		}
//		if (value < minValue){
//			minValue = value;
//			minIndex = data[i].row;
//		}
//		bool flag = false;
//		for (int p = -7; p <= 2; p++) {
//			if (value < pow(10, p * 1.0)) {
//				count[p + 7]++;
//				flag = true;
//				break;
//			}
//		}
//		if (!flag) {
//			count[10]++;
//		}
//	}
//	cout << "max value = " << maxValue <<", index = "<<maxIndex<< endl;
//	cout << "min value = " << minValue <<", index = "<<minIndex<< endl;
//	for (int i = 0; i < 11; ++i) {
//		cout << count[i] << " ";
//	}
//	cout << endl;
//	long tx;
//	cin >> tx;

	//for debug
//	dumpFile(data, bx, sol, "Q.txt", "d.txt", "ans.txt");
//	cout<<"linear solver result output done"<<endl;
//	long x1;
//	cin >> x1;
}

void SimPlPlace::linearSolverX() {
	//for debug
	vector<Triple> data;
	vector<double> sol;

	int n = numMoveNodes;

	vector<Triple>::iterator pre;
	data.clear();
	sol.clear();

	//solve x direction
	for (long i = 0; i < n; i++) {
		for (vector<Triple>::iterator t = B2B_MatrixX[i].begin(); t
				!= B2B_MatrixX[i].end(); ++t) {
			if (t == B2B_MatrixX[i].begin()) {
				pre = t;
				continue;
			}
			if ((*t).column == (*pre).column) {
				(*pre).element += (*t).element;
				t = B2B_MatrixX[i].erase(t);
				t--;
			} else {
				assert((*pre).row >= (*pre).column);
				assert((*pre).row < n);
				assert((*pre).column < n);
				//cout<<"push "<<(*pre).row<<" "<<(*pre).column<<" "<<(*pre).element<<endl;
				data.push_back((*pre));
				pre = t;
			}
		}
		assert(B2B_MatrixX[i].back().row == i);
		assert(B2B_MatrixX[i].back().column == i);
		data.push_back(B2B_MatrixX[i].back());
	}
	//eigen(numMoveNodes, data, Bx, sol);
	cgSolver(n, data, Bx, sol);

	for (int i = 0; i < numMoveNodes; ++i) {
		validNodes[i]->setCoordX(sol[i] - validNodes[i]->getWidth() / 2);
	}
}

void SimPlPlace::linearSolverY() {
	int n = numMoveNodes;
	vector<Triple> data;
	vector<double> sol;
	vector<Triple>::iterator pre;

	//solve y direction
	data.clear();
	sol.clear();
	for (long i = 0; i < n; i++) {
		for (vector<Triple>::iterator t = B2B_MatrixY[i].begin(); t
				!= B2B_MatrixY[i].end(); ++t) {
			if (t == B2B_MatrixY[i].begin()) {
				pre = t;
				continue;
			}
			if ((*t).column == (*pre).column) {
				(*pre).element += (*t).element;
				t = B2B_MatrixY[i].erase(t);
				t--;
			} else {
				assert((*pre).row >= (*pre).column);
				data.push_back((*pre));
				pre = t;
			}
		}
		assert(B2B_MatrixY[i].back().row == i);
		assert(B2B_MatrixY[i].back().column == i);
		data.push_back(B2B_MatrixY[i].back());
	}
	//eigen(numMoveNodes, data, By, sol);
	cgSolver(numMoveNodes, data, By, sol);
	for (int i = 0; i < numMoveNodes; ++i) {
		validNodes[i]->setCoordY(sol[i] - validNodes[i]->getHeight() / 2);
	}
}
