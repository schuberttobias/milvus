// Licensed to the LF AI & Data foundation under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership. The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "FieldMeta.h"
#include "pb/schema.pb.h"
#include "Consts.h"

namespace milvus {

static int64_t debug_id = START_USER_FIELDID;

class Schema {
 public:
    FieldId
    AddDebugField(const std::string& name, DataType data_type) {
        auto field_id = FieldId(debug_id);
        debug_id++;
        this->AddField(FieldName(name), field_id, data_type);
        return field_id;
    }

    // auto gen field_id for convenience
    FieldId
    AddDebugField(const std::string& name, DataType data_type, int64_t dim, std::optional<MetricType> metric_type) {
        auto field_id = FieldId(debug_id);
        debug_id++;
        auto field_meta = FieldMeta(FieldName(name), field_id, data_type, dim, metric_type);
        this->AddField(std::move(field_meta));
        return field_id;
    }

    // scalar type
    void
    AddField(const FieldName& name, const FieldId id, DataType data_type) {
        auto field_meta = FieldMeta(name, id, data_type);
        this->AddField(std::move(field_meta));
    }

    // string type
    void
    AddField(const FieldName& name, const FieldId id, DataType data_type, int64_t max_length_per_row) {
        auto field_meta = FieldMeta(name, id, data_type, max_length_per_row);
        this->AddField(std::move(field_meta));
    }

    // vector type
    void
    AddField(const FieldName& name,
             const FieldId id,
             DataType data_type,
             int64_t dim,
             std::optional<MetricType> metric_type) {
        auto field_meta = FieldMeta(name, id, data_type, dim, metric_type);
        this->AddField(std::move(field_meta));
    }

    void
    set_primary_field_id(FieldId field_id) {
        this->primary_field_id_opt_ = field_id;
    }

    auto
    begin() const {
        return fields_.begin();
    }

    auto
    end() const {
        return fields_.end();
    }

    int
    size() const {
        return fields_.size();
    }

    const FieldMeta&
    operator[](FieldId field_id) const {
        Assert(field_id.get() >= 0);
        AssertInfo(fields_.count(field_id), "Cannot find field_id");
        return fields_.at(field_id);
    }

    auto
    get_total_sizeof() const {
        return total_sizeof_;
    }

    FieldId
    get_field_id(const FieldName& field_name) const {
        AssertInfo(name_ids_.count(field_name), "Cannot find field_name");
        return name_ids_.at(field_name);
    }

    const std::unordered_map<FieldId, FieldMeta>&
    get_fields() const {
        return fields_;
    }

    const std::vector<FieldId>&
    get_field_ids() const {
        return field_ids_;
    }

    const FieldMeta&
    operator[](const FieldName& field_name) const {
        auto id_iter = name_ids_.find(field_name);
        AssertInfo(id_iter != name_ids_.end(), "Cannot find field_name: " + field_name.get());
        return fields_.at(id_iter->second);
    }

    std::optional<FieldId>
    get_primary_field_id() const {
        return primary_field_id_opt_;
    }

 public:
    static std::shared_ptr<Schema>
    ParseFrom(const milvus::proto::schema::CollectionSchema& schema_proto);

    void
    AddField(FieldMeta&& field_meta) {
        auto field_name = field_meta.get_name();
        auto field_id = field_meta.get_id();
        AssertInfo(!name_ids_.count(field_name), "duplicated field name");
        AssertInfo(!id_names_.count(field_id), "duplicated field id");
        name_ids_.emplace(field_name, field_id);
        id_names_.emplace(field_id, field_name);

        fields_.emplace(field_id, field_meta);
        field_ids_.emplace_back(field_id);

        auto field_sizeof = field_meta.get_sizeof();
        total_sizeof_ += field_sizeof;
    }

 private:
    int64_t debug_id = START_USER_FIELDID;
    std::vector<FieldId> field_ids_;

    // this is where data holds
    std::unordered_map<FieldId, FieldMeta> fields_;

    // a mapping for random access
    std::unordered_map<FieldName, FieldId> name_ids_;  // field_name -> field_id
    std::unordered_map<FieldId, FieldName> id_names_;  // field_id -> field_name

    int64_t total_sizeof_ = 0;
    std::optional<FieldId> primary_field_id_opt_;
};

using SchemaPtr = std::shared_ptr<Schema>;

}  // namespace milvus
