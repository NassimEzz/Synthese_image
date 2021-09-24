#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/shape.h>

NORI_NAMESPACE_BEGIN
class Instance {
public:
  Instance(const PropertyList &props) {}

  void activate() {
    if (!m_bsdf) {
      /* If no material was assigned, instantiate a diffuse BRDF */
      m_bsdf = static_cast<BSDF *>(
          NoriObjectFactory::createInstance("diffuse", PropertyList()));
    }
  }

  bool rayIntersect(Ray3f &ray, Intersection &its,
                    bool shadowRay = false) const {
    

    float txp = (0.5f - ray.o.x()) / ray.d.x();
    float txm = (-0.5f - ray.o.x()) / ray.d.x();
    float typ = (0.5f - ray.o.y()) / ray.d.y();
    float tym = (-0.5f - ray.o.y()) / ray.d.y();
    float tzp = (0.5f - ray.o.z()) / ray.d.z();
    float tzm = (-0.5f - ray.o.z()) / ray.d.z();
    
    float txnear = std::min({txp, txm});
    float txfar = std::max({txp, txm});
    float tynear = std::min({typ, tym});
    float tyfar = std::max({typ, tym});
    float tznear = std::min({tzp, tzm});
    float tzfar = std::max({tzp, tzm});

    float tnear = std::max({txnear, tynear, tznear});
    float tfar = std::min({txfar, tyfar, tzfar});

    if (tnear > tfar) 
    {
      return false;
    }

    if (ray.mint > tfar)
    {
      return false;
    }

    if (ray.maxt < tnear)
    {
      return false;
    }

    if (ray.mint < tnear)
    {
      if (shadowRay) {
        return true;
      }

      Normal3f n(0, 0, 0);

      if (tnear == txnear) {
        if (tnear + ray.o.x()/ray.d.x() > 0) {
          n.x() = -1;
        } else {
          n.x() = 1;
        }
      } else if (tnear == tynear) {
        if (tnear + ray.o.y()/ray.d.y() > 0) {
          n.y() = -1;
        } else {
          n.y() = 1;
        }
      } else {
        if (tnear + ray.o.z()/ray.d.z() > 0) {
          n.z() = -1;
        } else {
          n.z() = 1;
        }
      }

      updateRayAndHit(ray, its, tnear, n);
      return true;
    }

    if (ray.maxt > tfar)
    {
      if (shadowRay) {
        return true;
      }

      Normal3f n(0, 0, 0);

      if (tfar == txfar) {
        if (tfar + ray.o.x()/ray.d.x() > 0) {
          n.x() = 1;
        } else {
          n.x() = -1;
        }
      } else if (tfar == tyfar) {
        if (tfar + ray.o.y()/ray.d.y() > 0) {
          n.y() = 1;
        } else {
          n.y() = -1;
        }
      } else {
        if (tfar + ray.o.z()/ray.d.z() > 0) {
          n.z() = 1;
        } else {
          n.z() = -1;
        }
      }

      updateRayAndHit(ray, its, tfar, n);
      return true;   
    }

    return false;
  }

  /// Register a child object (e.g. a BSDF) with the mesh
  void addChild(NoriObject *obj) {
    switch (obj->getClassType()) {
    case EBSDF:
      if (m_bsdf)
        throw NoriException(
            "Instance: tried to register multiple BSDF instances!");
      m_bsdf = static_cast<BSDF *>(obj);
      break;

    case EEmitter: {
      Emitter *emitter = static_cast<Emitter *>(obj);
      if (m_emitter)
        throw NoriException(
            "Instance: tried to register multiple Emitter instances!");
      m_emitter = emitter;
    } break;

    default:
      throw NoriException("Instance::addChild(<%s>) is not supported!",
                          classTypeName(obj->getClassType()));
    }
  }

  std::string toString() const {
    return tfm::format(
        "Instance[\n"
        "emitter = %s\n"
        "bsdf = %s\n"
        "]",
        (m_emitter) ? indent(m_emitter->toString()) : std::string("null"),
        (m_bsdf) ? indent(m_bsdf->toString()) : std::string("null"));
  }

private:
  void updateRayAndHit(Ray3f &ray, Intersection &its, float t, Normal3f n) const {
    ray.maxt = its.t = t;
    its.p = ray(its.t);
    its.uv = Point2f(its.p.x(), its.p.z());
    its.bsdf = m_bsdf;
    its.emitter = m_emitter;
    its.geoFrame = its.shFrame = Frame(n);
  }

private:
  BSDF *m_bsdf = nullptr;       ///< BSDF of the surface
  Emitter *m_emitter = nullptr; ///< Associated emitter, if any
};

NORI_REGISTER_CLASS(Instance, "instance");
NORI_NAMESPACE_END
