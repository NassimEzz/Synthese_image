#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/shape.h>

NORI_NAMESPACE_BEGIN
class Sphere : public Shape {
public:
  Sphere(const PropertyList &props) {}

  void activate() {
    if (!m_bsdf) {
      /* If no material was assigned, instantiate a diffuse BRDF */
      m_bsdf = static_cast<BSDF *>(
          NoriObjectFactory::createInstance("diffuse", PropertyList()));
    }
  }

  bool rayIntersect(Ray3f &ray, Intersection &its,
                    bool shadowRay = false) const {
    const float r2 = 1;

    float a = (ray.d).dot(ray.d);
    float b = 2 * (ray.o).dot(ray.d);
    float c = (ray.o).dot(ray.o) - r2;
    float delta = b*b - 4 * a * c;

    if (delta < 0) {
      return false;
    }

    float t1 = (-b - sqrt(delta)) / 2 * a;
    float t2 = (-b + sqrt(delta)) / 2 * a;

    if (t1 > ray.maxt) {
      return false;
    }

    if (t1 > ray.mint) {
      if (shadowRay) {
        return true;
      }
      updateRayAndHit(ray, its, t1);
      return true;
    }

    if (t2 > ray.mint && t1 < ray.mint) {
      if (shadowRay) {
        return true;
      }
      updateRayAndHit(ray, its, t2);
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
            "Sphere: tried to register multiple BSDF instances!");
      m_bsdf = static_cast<BSDF *>(obj);
      break;

    case EEmitter: {
      Emitter *emitter = static_cast<Emitter *>(obj);
      if (m_emitter)
        throw NoriException(
            "Sphere: tried to register multiple Emitter instances!");
      m_emitter = emitter;
    } break;

    default:
      throw NoriException("Sphere::addChild(<%s>) is not supported!",
                          classTypeName(obj->getClassType()));
    }
  }

  std::string toString() const {
    return tfm::format(
        "Sphere[\n"
        "emitter = %s\n"
        "bsdf = %s\n"
        "]",
        (m_emitter) ? indent(m_emitter->toString()) : std::string("null"),
        (m_bsdf) ? indent(m_bsdf->toString()) : std::string("null"));
  }

private:
  void updateRayAndHit(Ray3f &ray, Intersection &its, float t) const {
    ray.maxt = its.t = t;
    its.p = ray(its.t);
    Normal3f n = its.p;
    its.uv = Point2f(its.p.x(), its.p.z());
    its.bsdf = m_bsdf;
    its.emitter = m_emitter;
    its.geoFrame = its.shFrame = Frame(n);
  }

private:
  BSDF *m_bsdf = nullptr;       ///< BSDF of the surface
  Emitter *m_emitter = nullptr; ///< Associated emitter, if any
};

NORI_REGISTER_CLASS(Sphere, "sphere");
NORI_NAMESPACE_END
