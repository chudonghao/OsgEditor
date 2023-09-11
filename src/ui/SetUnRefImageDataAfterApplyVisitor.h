//
// Created by HG on 2020/7/31.
//

#ifndef OSGEDITOR_172D6A9C2BF648EDB3F51A6FF187C73D_
#define OSGEDITOR_172D6A9C2BF648EDB3F51A6FF187C73D_

#include <iostream>
#include <osg/Drawable>
#include <osg/NodeVisitor>
#include <osg/Texture>

class SetUnRefImageDataAfterApplyVisitor : public osg::NodeVisitor {
 public:
  SetUnRefImageDataAfterApplyVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}

 protected:
  void apply(osg::Node &node) override {
    auto state_set = node.getStateSet();
    if (!state_set) {
      traverse(node);
      return;
    }
    auto &tal = state_set->getTextureAttributeList();
    for (auto &ta : tal) {
      for (auto &p : ta) {
        auto tex = p.second.first->asTexture();
        if (tex) {
          auto image = tex->getImage(GL_FRONT);
          if (image) {
            std::cout << node.getName() << " " << image->getFileName() << std::endl;
          }
          tex->setUnRefImageDataAfterApply(false);
        }
      }
    }
    traverse(node);
  }
};

#endif  // OSGEDITOR_172D6A9C2BF648EDB3F51A6FF187C73D_
