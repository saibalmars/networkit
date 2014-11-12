/*
 *
 */

#include "AttributeBlender.h"

NetworKit::AttributeBlender::AttributeBlender(const NetworKit::Graph &G, const std::vector< double > &attribute0, const std::vector< double > &attribute1, const std::vector< bool > &selection) :
G(G), attribute0(attribute0), attribute1(attribute1), selection(selection), hasAttribute(false) {}

void NetworKit::AttributeBlender::run() {
	blendedAttribute.resize(G.upperEdgeIdBound());

	G.parallelForEdges([&](node u, node v, edgeid eid) {
		blendedAttribute[eid] = (selection[eid] ? attribute1[eid] : attribute0[eid]);
	});

	hasAttribute = true;
}

std::vector< double > NetworKit::AttributeBlender::getAttribute() {
	if (!hasAttribute) throw std::runtime_error("Error: Run must called first");

	hasAttribute = false;

	return std::move(blendedAttribute);
}
