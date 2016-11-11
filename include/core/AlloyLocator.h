/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ALLOYPOINTLOCATOR_H_
#define ALLOYPOINTLOCATOR_H_
#include "libkdtree/kdtree.h"
#include "AlloyMath.h"
#include "AlloyVector.h"
#include "AlloyArray.h"
#include "nanoflann.h"

namespace aly {
bool SANITY_CHECK_LOCATOR();
template<class T, int C> struct xvec: public vec<T, C> {
	typedef T value_type;
	int index;
	xvec() :
			vec<T, C>(), index(-1) {
	}
	xvec(const xvec<T, C>& pt) :
			vec<T, C>(pt), index(pt.index) {

	}
	xvec(const vec<T, C>& pos, int index) :
			vec<T, C>(pos), index(index) {
	}
	double distance(const xvec<T, C> &node) const {
		vec<T, C> diff = (*this) - node;
		return aly::max(diff);
	}
	bool operator ==(const xvec<T, C> & r) const {
		return (index == r.index);
	}
	bool operator !=(const xvec<T, C> & r) const {
		return (index != r.index);
	}
};

typedef xvec<float, 2> float2i;
typedef xvec<float, 3> float3i;
typedef xvec<float, 4> float4i;

typedef xvec<double, 2> double2i;
typedef xvec<double, 3> double3i;
typedef xvec<double, 4> double4i;

template<class T, int C> class Locator {
protected:
	typedef libkdtree::KDTree<C, xvec<T, C> > LocatorType;
	LocatorType locator;
	int indexCount = 0;
public:
	static const xvec<T, C> NO_POINT_FOUND;
	Locator() {
	}
	void clear() {
		indexCount = 0;
		locator.clear();
	}
	size_t size() const {
		return locator.size();
	}
	void insert(const xvec<T, C>& pt) {
		indexCount = std::max(indexCount, (int) (pt.index + 1));
		locator.insert(pt);
	}
	void erase(const xvec<T, C>& pt) {
		locator.erase(pt);
	}
	void eraseExact(const xvec<T, C>& pt) {
		locator.erase_exact(pt);
	}
	void erase(const vec<T, C>& pt) {
		locator.erase(xvec<T, C>(pt, -1));
	}
	void erase(const std::vector<xvec<T, C>>& pts) {
		locator.erase(pts.begin(), pts.end());
	}
	int insert(const vec<T, C>& pt) {
		xvec<T, C> pti(pt, indexCount++);
		locator.insert(pti);
		return pti.index;
	}
	void insert(const std::vector<vec<T, C>>& pts) {
		std::vector<xvec<T, C>> ptsi;
		ptsi.reserve(pts.size());
		for (vec<T, C> pt : pts) {
			xvec<T, C> pti(pt, indexCount++);
			ptsi.push_back(pti);
		}
		locator.insert(ptsi.begin(), ptsi.end());
	}
	void insert(const Vector<T, C>& pts) {
		insert(pts.data);
	}
	Locator(const std::vector<vec<T, C>>& data) {
		insert(data);
	}
	Locator(const Vector<T, C>& data) {
		insert(data.data);
	}
	void insert(const std::vector<xvec<T, C>>& ptsi) {
		for (xvec<T, C> pt : ptsi) {
			indexCount = std::max(indexCount, (pt.index + 1));
		}
		locator.insert(ptsi.begin(), ptsi.end());
	}
	xvec<T, C> closest(vec<T, C> pt, T maxDistance) const {
		xvec<T, C> query(pt, -1);
		auto result = locator.find_nearest(query, maxDistance);
		if (result.first != locator.end()) {
			return *(result.first);
		} else {
			return NO_POINT_FOUND;
		}
	}
	xvec<T, C> closestExact(xvec<T, C> query) const {
		auto result = locator.find_exact(query);
		if (result != locator.end()) {
			return *(result);
		} else {
			return NO_POINT_FOUND;
		}
	}
	xvec<T, C> closest(vec<T, C> pt) const {
		xvec<T, C> query(pt, -1);
		auto result = locator.find_nearest(query);
		if (result.first != locator.end()) {
			return *(result.first);
		} else {
			return NO_POINT_FOUND;
		}
	}

	void closest(vec<T, C> pt, T maxDistance,
			std::vector<xvec<T, C>>& pts) const {
		xvec<T, C> query(pt, -1);
		pts.clear();
		std::vector<xvec<T, C>> tmp;
		locator.find_within_range(query, maxDistance,
				std::back_insert_iterator<std::vector<xvec<T, C>> >(tmp));
		T distSqr = maxDistance * maxDistance;
		pts.clear();
		pts.reserve(tmp.size());
		//Nearest not guaranteed to be strictly less than maxDistance under L2 norm. Check that each element is, then sort.
		for (xvec<T, C>& val : tmp) {
			if (distanceSqr(pt, val) <= distSqr) {
				pts.push_back(val);
			}
		}
		std::sort(pts.begin(), pts.end(),
				[=](const xvec<T, C>& a, const xvec<T, C>& b) {
					return (distanceSqr(pt, a) < distanceSqr(pt, b));
				});

	}
	void closest(vec<T, C> pt, T maxDistance,
			std::vector<std::pair<xvec<T, C>, T>>& pts) const {
		xvec<T, C> query(pt, -1);
		pts.clear();
		std::vector<xvec<T, C>> tmp;
		locator.find_within_range(query, maxDistance,
				std::back_insert_iterator<std::vector<xvec<T, C>> >(tmp));
		pts.clear();
		pts.reserve(tmp.size());
		//Nearest not guaranteed to be strictly less than maxDistance under L2 norm. Check that each element is, then sort.
		for (xvec<T, C>& val : tmp) {
			T d = distance(pt, val);
			if (d <= maxDistance) {
				pts.push_back(std::pair<xvec<T, C>, T>(val, d));
			}
		}
		std::sort(pts.begin(), pts.end(),
				[=](const std::pair<xvec<T, C>, T>& a, const std::pair<xvec<T, C>, T>& b) {
					return (a.second< b.second);
				});

	}
};
template<class T, int C> const xvec<T, C> Locator<T, C>::NO_POINT_FOUND = xvec<
		T, C>(vec<T, C>(std::numeric_limits<T>::max()), -1);

namespace detail {
template<typename T = double, int C = -1, class Distance = nanoflann::metric_L2,
		typename IndexType = size_t>
struct KdTreeVectorAdapter {
	typedef Vector<T, C> VectorOfVectorsType;
	typedef KdTreeVectorAdapter<T, C, Distance> self_t;
	typedef typename Distance::template traits<T, self_t>::distance_t metric_t;
	typedef nanoflann::KDTreeSingleIndexAdaptor<metric_t, self_t, C, IndexType> index_t;
	index_t* index; //! The kd-tree index for the user to call its methods as usual with any other FLANN index.
	vec<T, C> minPt;
	vec<T, C> maxPt;
	KdTreeVectorAdapter(const VectorOfVectorsType &mat,
			const int leaf_max_size = 16) :
			m_data(mat) {
		assert(mat.size() != 0 && mat[0].size() != 0);
		index = new index_t(C, *this,
				nanoflann::KDTreeSingleIndexAdaptorParams(leaf_max_size));
		minPt = mat.min();
		maxPt = mat.max();
		index->buildIndex();
	}

	~KdTreeVectorAdapter() {
		delete index;
	}
	const VectorOfVectorsType &m_data;
	/** Query for the \a num_closest closest points to a given point (entered as query_point[0:dim-1]).
	 *  Note that this is a short-cut method for index->findNeighbors().
	 *  The user can also call index->... methods as desired.
	 * \note nChecks_IGNORED is ignored but kept for compatibility with the original FLANN interface.
	 */
	inline void query(const T *query_point, const size_t num_closest,
			IndexType *out_indices, T *out_distances_sq,
			const int nChecks_IGNORED = 10) const {
		nanoflann::KNNResultSet<typename VectorOfVectorsType::Scalar, IndexType> resultSet(
				num_closest);
		resultSet.init(out_indices, out_distances_sq);
		index->findNeighbors(resultSet, query_point, nanoflann::SearchParams());
	}
	const self_t & derived() const {
		return *this;
	}
	self_t & derived() {
		return *this;
	}
	// Must return the number of data points
	inline size_t kdtree_get_point_count() const {
		return m_data.size();
	}
	// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
	inline T kdtree_distance(const T *p1, const size_t idx_p2,
			size_t size) const {
		T s = 0;
		for (size_t i = 0; i < size; i++) {
			const T d = p1[i] - m_data[idx_p2][i];
			s += d * d;
		}
		return s;
	}
	// Returns the dim'th component of the idx'th point in the class:
	inline T kdtree_get_pt(const size_t idx, size_t dim) const {
		return m_data[idx][dim];
	}
	template<class BBOX>
	bool kdtree_get_bbox(BBOX & bb) const {
		for (int c = 0; c < C; c++) {
			bb[c].low = minPt[c];
			bb[c].high = maxPt[c];
		}
		return true;
	}
};
// end of KdTreeVectorAdapter

template<typename T = float, int C = -1, class Distance = nanoflann::metric_L2,
		typename IndexType = size_t> struct KdTreeArrayAdapter {
	typedef std::vector<Array<T, C>> VectorOfVectorsType;
	typedef KdTreeArrayAdapter<T, C, Distance> self_t;
	typedef typename Distance::template traits<T, self_t>::distance_t metric_t;
	typedef nanoflann::KDTreeSingleIndexAdaptor<metric_t, self_t, C, IndexType> index_t;
	index_t* index; //! The kd-tree index for the user to call its methods as usual with any other FLANN index.
	KdTreeArrayAdapter(const VectorOfVectorsType &mat, const int leaf_max_size =
			16) :
			m_data(mat) {
		assert(mat.size() != 0 && mat[0].size() != 0);
		index = new index_t(C, *this,
				nanoflann::KDTreeSingleIndexAdaptorParams(leaf_max_size));
		index->buildIndex();
	}

	~KdTreeArrayAdapter() {
		delete index;
	}
	const VectorOfVectorsType &m_data;
	/** Query for the \a num_closest closest points to a given point (entered as query_point[0:dim-1]).
	 *  Note that this is a short-cut method for index->findNeighbors().
	 *  The user can also call index->... methods as desired.
	 * \note nChecks_IGNORED is ignored but kept for compatibility with the original FLANN interface.
	 */
	inline void query(const T *query_point, const size_t num_closest,
			IndexType *out_indices, T *out_distances_sq,
			const int nChecks_IGNORED = 10) const {
		nanoflann::KNNResultSet<typename VectorOfVectorsType::Scalar, IndexType> resultSet(
				num_closest);
		resultSet.init(out_indices, out_distances_sq);
		index->findNeighbors(resultSet, query_point, nanoflann::SearchParams());
	}
	const self_t & derived() const {
		return *this;
	}
	self_t & derived() {
		return *this;
	}
	// Must return the number of data points
	inline size_t kdtree_get_point_count() const {
		return m_data.size();
	}
	// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
	inline T kdtree_distance(const T *p1, const size_t idx_p2,
			size_t size) const {
		T s = 0;
		for (size_t i = 0; i < size; i++) {
			const T d = p1[i] - m_data[idx_p2][i];
			s += d * d;
		}
		return s;
	}
	// Returns the dim'th component of the idx'th point in the class:
	inline T kdtree_get_pt(const size_t idx, size_t dim) const {
		return m_data[idx][dim];
	}
	template<class BBOX>
	bool kdtree_get_bbox(BBOX & bb) const {
		return false;
	}
};
}
// end of KdTreeArrayAdapter
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const xvec<T, 2> & v) {
	return ss << "(" << v.x << ", " << v.y << ": " << v.index << ")";
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const xvec<T, 3> & v) {
	return ss << "(" << v.x << ", " << v.y << ", " << v.z << ": " << v.index
			<< ")";
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const xvec<T, 4> & v) {
	return ss << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ": "
			<< v.index << ")";
}
//FLANN Matcher does not allow the insertion of points! All data must be provided up front.
template<class T, int C> class Matcher {
protected:
	detail::KdTreeArrayAdapter<T, C, nanoflann::metric_L2, size_t> locator;
public:
	static const size_t NO_POINT_FOUND;
	Matcher(const std::vector<Array<T, C>>& data, int maxDepth = 16) :
			locator(data, maxDepth) {
	}
	void closest(const Array<T, C>& pt, T maxDistance,
			std::vector<size_t>& matches) const {
		std::vector<std::pair<size_t, T> > ret_matches;
		nanoflann::SearchParams params;
		const size_t nMatches = locator.index->radiusSearch(pt.data(),
				maxDistance * maxDistance, ret_matches, params);
		matches.clear();
		matches.reserve(nMatches);
		for (size_t i = 0; i < nMatches; i++) {
			matches.push_back(ret_matches[i].first);
		}
	}
	void closest(const Array<T, C>& pt, T maxDistance,
			std::vector<std::pair<size_t, T>>& matches) const {
		matches.clear();
		nanoflann::SearchParams params;
		locator.index->radiusSearch(pt.data(), maxDistance * maxDistance,
				matches, params);
		for (std::pair<size_t, T>& pr : matches) {
			pr.second = std::sqrt(pr.second);
		}
	}
	void closest(const Array<T, C>& pt, int kNN,
			std::vector<size_t>& matches) const {
		std::vector<size_t> ret_index(kNN, NO_POINT_FOUND);
		std::vector<T> out_dist_sqr(kNN);
		locator.index->knnSearch(pt.data(), kNN, ret_index.data(),
				out_dist_sqr.data());
		matches.clear();
		matches.reserve(kNN);
		for (int i = 0; i < kNN; i++) {
			if (ret_index[i] != NO_POINT_FOUND) {
				matches.push_back(ret_index[i]);
			}
		}
	}
	size_t closest(const Array<T, C>& pt) const {
		std::vector<size_t> ret_index(1, NO_POINT_FOUND);
		std::vector<T> out_dist_sqr(1);
		locator.index->knnSearch(pt.data(), 1, ret_index.data(),
				out_dist_sqr.data());
		return ret_index[0];
	}
	size_t closest(const Array<T, C>& pt, T maxDistance) const {
		std::vector<std::pair<size_t, T> > ret_matches;
		nanoflann::SearchParams params;
		const size_t nMatches = locator.index->radiusSearch(pt.data(),
				maxDistance * maxDistance, ret_matches, params);
		if (nMatches > 0) {
			return ret_matches[0].first;
		} else {
			return NO_POINT_FOUND;
		}
	}
	void closest(const Array<T, C>& pt, int kNN,
			std::vector<std::pair<size_t, T>>& matches) const {
		std::vector<size_t> ret_index(kNN, NO_POINT_FOUND);
		std::vector<T> out_dist_sqr(kNN);
		locator.index->knnSearch(pt.data(), kNN, ret_index.data(),
				out_dist_sqr.data());
		matches.clear();
		matches.reserve(kNN);
		for (int i = 0; i < kNN; i++) {
			if (ret_index[i] != NO_POINT_FOUND) {
				matches.push_back(
						std::pair<size_t, T>(ret_index[i],
								std::sqrt(out_dist_sqr[i])));
			}
		}
	}
};
template<class T, int C> const size_t Matcher<T, C>::NO_POINT_FOUND =
		std::numeric_limits<size_t>::max();
//FLANN Matcher does not allow the insertion of points! All data must be provided up front.
template<class T, int C> class MatcherVec {
protected:
	detail::KdTreeVectorAdapter<T, C, nanoflann::metric_L2, size_t> locator;
public:
	static const size_t NO_POINT_FOUND;
	MatcherVec(const Vector<T, C>& data, int maxDepth = 16) :
			locator(data, maxDepth) {
	}
	void closest(const vec<T, C>& pt, T maxDistance,
			std::vector<size_t>& matches) const {
		std::vector<std::pair<size_t, T> > ret_matches;
		nanoflann::SearchParams params;
		const size_t nMatches = locator.index->radiusSearch(&pt[0],
				maxDistance * maxDistance, ret_matches, params);
		matches.clear();
		matches.reserve(nMatches);
		for (size_t i = 0; i < nMatches; i++) {
			matches.push_back(ret_matches[i].first);
		}
	}
	void closest(const vec<T, C>& pt, T maxDistance,
			std::vector<std::pair<size_t, T>>& matches) const {
		matches.clear();
		nanoflann::SearchParams params;
		locator.index->radiusSearch(&pt[0], maxDistance * maxDistance, matches,
				params);
		for (std::pair<size_t, T>& pr : matches) {
			pr.second = std::sqrt(pr.second);
		}
	}
	void closest(const vec<T, C>& pt, int kNN,
			std::vector<size_t>& matches) const {
		std::vector<size_t> ret_index(kNN, NO_POINT_FOUND);
		std::vector<T> out_dist_sqr(kNN);
		locator.index->knnSearch(&pt[0], kNN, ret_index.data(),
				out_dist_sqr.data());
		matches.clear();
		matches.reserve(kNN);
		for (int i = 0; i < kNN; i++) {
			if (ret_index[i] != NO_POINT_FOUND) {
				matches.push_back(ret_index[i]);
			}
		}
	}
	void closest(const vec<T, C>& pt, int kNN,
			std::vector<std::pair<size_t, T>>& matches) const {
		std::vector<size_t> ret_index(kNN, NO_POINT_FOUND);
		std::vector<T> out_dist_sqr(kNN);
		locator.index->knnSearch(&pt[0], kNN, ret_index.data(),
				out_dist_sqr.data());
		matches.clear();
		matches.reserve(kNN);
		for (int i = 0; i < kNN; i++) {
			if (ret_index[i] != NO_POINT_FOUND) {
				matches.push_back(
						std::pair<size_t, T>(ret_index[i],
								std::sqrt(out_dist_sqr[i])));
			}
		}
	}
	size_t closest(const vec<T, C>& pt) const {
		std::vector<size_t> ret_index(1, NO_POINT_FOUND);
		std::vector<T> out_dist_sqr(1);
		locator.index->knnSearch(&pt[0], 1, ret_index.data(),
				out_dist_sqr.data());
		return ret_index[0];
	}
	size_t closest(const vec<T, C>& pt, T maxDistance) const {
		std::vector<std::pair<size_t, T> > ret_matches;
		nanoflann::SearchParams params;
		const size_t nMatches = locator.index->radiusSearch(&pt[0],
				maxDistance * maxDistance, ret_matches, params);
		if (nMatches > 0) {
			return ret_matches[0].first;
		} else {
			return NO_POINT_FOUND;
		}
	}
};
template<class T, int C> const size_t MatcherVec<T, C>::NO_POINT_FOUND =
		std::numeric_limits<size_t>::max();

typedef Locator<float, 2> Locator2f;
typedef Locator<float, 3> Locator3f;
typedef Locator<float, 4> Locator4f;

typedef Locator<double, 2> Locator2d;
typedef Locator<double, 3> Locator3d;
typedef Locator<double, 4> Locator4d;

typedef MatcherVec<float, 2> Matcher2f;
typedef MatcherVec<float, 3> Matcher3f;
typedef MatcherVec<float, 4> Matcher4f;
typedef Matcher<float, 256> Matcher256f;
typedef Matcher<float, 512> Matcher512f;
typedef Matcher<float, 1024> Matcher1024f;

typedef MatcherVec<double, 2> Matcher2d;
typedef MatcherVec<double, 3> Matcher3d;
typedef MatcherVec<double, 4> Matcher4d;
typedef Matcher<double, 256> Matcher256d;
typedef Matcher<double, 512> Matcher512d;
typedef Matcher<double, 1024> Matcher1024d;

}
#endif
