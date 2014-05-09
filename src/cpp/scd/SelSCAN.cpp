#include "SelSCAN.h"

#include <queue>

namespace NetworKit {

SelSCAN::SelSCAN(const Graph& G, count kappa, double epsilon) : SelectiveCommunityDetector(G), kappa(kappa), epsilon(epsilon), algebraicDistance(NULL) {
	/**
	 * expresses neighborhood overlap
	 */
	auto neighborhoodOverlap = [&](node u, node v) -> double {
		auto Nu = G.neighbors(u);
		auto Nv = G.neighbors(v);
		Nu.push_back(u);
		Nv.push_back(v);
		std::set<node> intersect;
		set_intersection(Nu.begin(), Nu.end(), Nv.begin(), Nv.end(), std::inserter(intersect, intersect.begin()));

		return 1 - (intersect.size() / sqrt(Nu.size() * Nv.size()));
	};

	dist = neighborhoodOverlap;

}

SelSCAN::SelSCAN(const Graph& G, count kappa, double epsilon, AlgebraicDistance& ad) : SelectiveCommunityDetector(G), kappa(kappa), epsilon(epsilon), algebraicDistance(&ad) {
	auto algDist = [&](node u, node v) -> double {
		return algebraicDistance->distance(u, v);
	};

	dist = algDist;
}

void SelSCAN::run(std::set<unsigned int>& seeds) {

	/**
	 * @return the epsilon-neighborhood of a node
	 */
	auto epsilonNeighborhood = [&](node u) {
		std::set<node> N;
		G.forNeighborsOf(u, [&](node v) {
			if (dist(u, v) < epsilon) {
				N.insert(v);
			}
		});
		return N;
	};

	/**
	 * determines if a node is a core
	 */
	auto core = [&](node u) {
		return (epsilonNeighborhood(u).size() >= kappa);
	};





	// map from node to community label
	std::unordered_map<node, index> eta;
	index label = 0;	// current community label

	auto newLabel = [&]() {
		label += 1;
		return label;
	};


	/**
	* breadth-first search which appends neighbors to the community
	* and cores to the queue.
	*/
	auto coreSearch = [&](std::queue<node>& Q, index label) {
		while (!Q.empty()) {
			node x = Q.front(); Q.pop();
			eta[x] = label;	// add to community
			if (core(x)) {
				for (node y : epsilonNeighborhood(x)) {
					assert ((eta.find(y) == eta.end()) || (eta[y] == none));	// not assigned or outlier
					Q.push(y);
				}
			}
		}

	};


	for (auto s: seeds) {
		if (eta.find(s) == eta.end()) {	// s not yet assigned to community
			std::queue<node> Q;
			if (core(s)) {
				eta[s] = newLabel();
				Q.push(s);
				coreSearch(Q, eta[s]);
			} else {
				// find core with minimum distance to s in epsilon neighborhood of s
				node minC = none;
				double minD = std::numeric_limits<double>::max();
				for (node c : epsilonNeighborhood(s)) {
					if (core(c)) {
						if (dist(s, c) < minD) {
							minD = dist(s, c);
							minC = c;
						}
					}
				}

				if (minC == none) { 	// no core in neighborhood
					eta[s] = none; 	// mark s as outlier
				} else {
					eta[s] = newLabel();
					Q.push(s);
					coreSearch(Q, eta[s]);
				}
			} //
		}

	} // end community discovery

	// extract sets from labels
	for (auto s : seeds) {
		index ls = eta[s];
		std::set<node> community;
		for (auto kv : eta) {
			if (kv.second == ls) {
				community.insert(kv.first);
			}
		}
		result[s] = community;
	}

}


}
