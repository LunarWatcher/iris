#pragma once

#include <map>
#include <memory>
#include <string>

#include "node.hpp"
#include "statistics.hpp"

namespace iris {

/*!
 * \brief The main iris Template.
 */
struct Template {
    BlockNode root;
    std::string content;
    /**
     * Either the absolute path to the file that this template is for, or a user-provided identifier if this is a
     * top-level render done without context (or an empty string in that case).
     *
     * This can be used to pin a render stack back to a source file, even when the source file isn't actually a
     * file.
     * This may always be a path depending on your implementation; if you always provide absolute paths, this will only
     * ever contain absolute paths.
     */
    std::string pathOrIdentifier;
    std::map<std::string, std::shared_ptr<BlockStatementNode>> block_storage;

    explicit Template() {}
    explicit Template(std::string content, std::string pathOrIdentifier)
        : content(std::move(content)),
          pathOrIdentifier(std::move(pathOrIdentifier))
    {}

    /// Return number of variables (total number, not distinct ones) in the template
    size_t count_variables() const {
        auto statistic_visitor = StatisticsVisitor();
        root.accept(statistic_visitor);
        return statistic_visitor.variable_counter;
    }
};

using TemplateStorage = std::map<std::string, Template>;

} // namespace iris
