#ifndef slic3r_SmallAreaInfillFlowCompensator_hpp_
#define slic3r_SmallAreaInfillFlowCompensator_hpp_

#include "../libslic3r.h"
#include "../PrintConfig.hpp"
#include "../ExtrusionEntity.hpp"
#include "spline/spline.h"

namespace Slic3r {


class SmallAreaInfillFlowCompensator
{
private:
    // Model points
    std::vector<double> extrusionLengths;
    std::vector<double> flowCompensationFactors;

    tk::spline flowModel;
    
private:
    double flow_comp_model(const double line_length);

    double max_modified_length() { return extrusionLengths.back(); }

    void check_model_parameter_correctness();

    void read_config_parameters(const Slic3r::FullPrintConfig& config);

public:
    explicit SmallAreaInfillFlowCompensator(const Slic3r::FullPrintConfig& config);

    double modify_flow(const double line_length, const double dE, const ExtrusionRole role);
};

} // namespace Slic3r

#endif /* slic3r_SmallAreaInfillFlowCompensator_hpp_ */
