#include "SmallAreaInfillFlowCompensator.hpp"

#include "../libslic3r.h"

namespace Slic3r{

bool nearly_equal_floating_point(double a, double b) {
    return std::nextafter(a, std::numeric_limits<double>::lowest()) <= b &&
        std::nextafter(a, std::numeric_limits<double>::max()) >= b;
}

void SmallAreaInfillFlowCompensator::check_model_parameter_correctness() {
    if (extrusionLengths.empty()) {
        throw Slic3r::InvalidArgument(
            "Small area infill compensation model is misconfigured: no lengths have been set"
        );
    }
    if (flowCompensationFactors.empty()) {
        throw Slic3r::InvalidArgument(
            "Small area infill compensation model is misconfigured: no compensation factors have been set"
        );
    }

    if (extrusionLengths.size() != flowCompensationFactors.size()) {
        throw Slic3r::InvalidArgument("Small area infill compensation model is misconfigured: "
                                      "Different size of lengths and compensation factors");
    }

   if (!nearly_equal_floating_point(extrusionLengths[0], 0.0)) {
        throw Slic3r::InvalidArgument(
            "First extrusion length for small area infill compensation model must be 0"
        );
    }

    for (int i = 1; i < extrusionLengths.size(); i++) {
        if (nearly_equal_floating_point(extrusionLengths[i], 0.0)) {
            throw Slic3r::InvalidArgument("Only the first extrusion length for small area "
                                            "infill compensation model can be 0");
        }
        if (extrusionLengths[i] <= extrusionLengths[i - 1]) {
            throw Slic3r::InvalidArgument(
                "Extrusion lengths for subsequent points must be in increasing order"
            );
        }
    }

    if (!nearly_equal_floating_point(flowCompensationFactors.back(), 1.0)) {
        throw Slic3r::InvalidArgument(
            "Final compensation factor for small area infill flow compensation model must be 1.0"
        );
    }
}

void SmallAreaInfillFlowCompensator::read_config_parameters(const Slic3r::FullPrintConfig& config) {
    for (auto &line : config.small_area_infill_flow_compensation_model.values) {
        std::istringstream iss(line);
        std::string value_str;
        double extrusion_length = 0.0;

        if (std::getline(iss, value_str, ',')) {
            try {
                extrusion_length = std::stod(value_str);
                if (std::getline(iss, value_str, ',')) {
                    extrusionLengths.push_back(extrusion_length);
                    flowCompensationFactors.push_back(std::stod(value_str));
                }
            } catch (...) {
                std::stringstream ss;
                ss << "Error parsing data point in small area infill compensation model:" << line
                   << std::endl;

                throw Slic3r::InvalidArgument(ss.str());
            }
        }
    }
}


SmallAreaInfillFlowCompensator::SmallAreaInfillFlowCompensator(const Slic3r::FullPrintConfig& config) {
    read_config_parameters(config);
    check_model_parameter_correctness();
    flowModel.set_points(extrusionLengths, flowCompensationFactors);
}

double SmallAreaInfillFlowCompensator::flow_comp_model(const double line_length) {
    if (line_length == 0 || line_length > max_modified_length()) {
        return 1.0;
    }

    return flowModel(line_length);
}

double SmallAreaInfillFlowCompensator::modify_flow(
    const double line_length, const double dE, const ExtrusionRole role
) {
    if (role == ExtrusionRole::erSolidInfill || role == ExtrusionRole::erTopSolidInfill) {
        return dE * flow_comp_model(line_length);
    }

    return dE;
}

} // namespace Slic3r