// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// This file has been generated from the Jinja2 template in
// third_party/WebKit/Source/bindings/templates/union_container.cpp.tmpl

// clang-format off
#include "node_or_node_list.h"

#include "bindings/core/v8/ToV8ForCore.h"
#include "bindings/core/v8/V8Node.h"
#include "bindings/core/v8/V8NodeList.h"
#include "core/dom/NameNodeList.h"
#include "core/dom/NodeList.h"
#include "core/dom/StaticNodeList.h"
#include "core/html/LabelsNodeList.h"

namespace blink {

NodeOrNodeList::NodeOrNodeList() : type_(SpecificTypeNone) {}

Node* NodeOrNodeList::getAsNode() const {
  DCHECK(isNode());
  return node_;
}

void NodeOrNodeList::setNode(Node* value) {
  DCHECK(isNull());
  node_ = value;
  type_ = SpecificTypeNode;
}

NodeOrNodeList NodeOrNodeList::fromNode(Node* value) {
  NodeOrNodeList container;
  container.setNode(value);
  return container;
}

NodeList* NodeOrNodeList::getAsNodeList() const {
  DCHECK(isNodeList());
  return node_list_;
}

void NodeOrNodeList::setNodeList(NodeList* value) {
  DCHECK(isNull());
  node_list_ = value;
  type_ = SpecificTypeNodeList;
}

NodeOrNodeList NodeOrNodeList::fromNodeList(NodeList* value) {
  NodeOrNodeList container;
  container.setNodeList(value);
  return container;
}

NodeOrNodeList::NodeOrNodeList(const NodeOrNodeList&) = default;
NodeOrNodeList::~NodeOrNodeList() = default;
NodeOrNodeList& NodeOrNodeList::operator=(const NodeOrNodeList&) = default;

DEFINE_TRACE(NodeOrNodeList) {
  visitor->Trace(node_);
  visitor->Trace(node_list_);
}

void V8NodeOrNodeList::ToImpl(v8::Isolate* isolate, v8::Local<v8::Value> v8Value, NodeOrNodeList& impl, UnionTypeConversionMode conversionMode, ExceptionState& exceptionState) {
  if (v8Value.IsEmpty())
    return;

  if (conversionMode == UnionTypeConversionMode::kNullable && IsUndefinedOrNull(v8Value))
    return;

  if (V8Node::hasInstance(v8Value, isolate)) {
    Node* cppValue = V8Node::ToImpl(v8::Local<v8::Object>::Cast(v8Value));
    impl.setNode(cppValue);
    return;
  }

  if (V8NodeList::hasInstance(v8Value, isolate)) {
    NodeList* cppValue = V8NodeList::ToImpl(v8::Local<v8::Object>::Cast(v8Value));
    impl.setNodeList(cppValue);
    return;
  }

  exceptionState.ThrowTypeError("The provided value is not of type '(Node or NodeList)'");
}

v8::Local<v8::Value> ToV8(const NodeOrNodeList& impl, v8::Local<v8::Object> creationContext, v8::Isolate* isolate) {
  switch (impl.type_) {
    case NodeOrNodeList::SpecificTypeNone:
      return v8::Null(isolate);
    case NodeOrNodeList::SpecificTypeNode:
      return ToV8(impl.getAsNode(), creationContext, isolate);
    case NodeOrNodeList::SpecificTypeNodeList:
      return ToV8(impl.getAsNodeList(), creationContext, isolate);
    default:
      NOTREACHED();
  }
  return v8::Local<v8::Value>();
}

NodeOrNodeList NativeValueTraits<NodeOrNodeList>::NativeValue(v8::Isolate* isolate, v8::Local<v8::Value> value, ExceptionState& exceptionState) {
  NodeOrNodeList impl;
  V8NodeOrNodeList::ToImpl(isolate, value, impl, UnionTypeConversionMode::kNotNullable, exceptionState);
  return impl;
}

}  // namespace blink
