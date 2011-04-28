#include "Vector3f.h"

class PLANE {
public:
    float equation[4];
    Vector3f origin;
    Vector3f normal;
    PLANE(const Vector3f& origin, const Vector3f& normal);
    PLANE(const Vector3f& p1, const Vector3f& p2, const Vector3f& p3);
    bool isFrontFacingTo(const Vector3f& direction) const;
    double signedDistanceTo(const Vector3f& point) const;
};

class CollisionPacket {
public:
    Vector3f eRadius; // ellipsoid radius
    // Information about the move being requested: (in R3)
    Vector3f R3Velocity;
    Vector3f R3Position;
    // Information about the move being requested: (in eSpace)
    Vector3f velocity;
    Vector3f normalizedVelocity;
    Vector3f basePoint;
    // Hit information
    bool foundCollision;
    double nearestDistance;
    Vector3f intersectionPoint;
};

PLANE::PLANE(const Vector3f& origin, const Vector3f& normal) {
    this->normal = normal;
    this->origin = origin;
    equation[0] = normal.x;
    equation[1] = normal.y;
    equation[2] = normal.z;
    equation[3] = -(normal.x*origin.x+normal.y*origin.y+normal.z*origin.z);
}
// Construct from triangle:
PLANE::PLANE(const Vector3f& p1,const Vector3f& p2,const Vector3f& p3)
{
    normal = (p2-p1).cross(p3-p1);
    normal.normalize();
    origin = p1;
    equation[0] = normal.x;
    equation[1] = normal.y;
    equation[2] = normal.z;
    equation[3] = -(normal.x*origin.x+normal.y*origin.y+normal.z*origin.z);
}

bool PLANE::isFrontFacingTo(const Vector3f& direction) const {
    double dot = normal.dot(direction);
    return (dot <= 0);
}

double PLANE::signedDistanceTo(const Vector3f& point) const {
    return (point.dot(normal)) + equation[3];
}


typedef unsigned int uint32;
#define in(a) ((uint32&) a)
bool checkPointInTriangle(const Vector3f& point,const Vector3f& pa,const Vector3f& pb, const Vector3f& pc)
{
    Vector3f e10=pb-pa;
    Vector3f e20=pc-pa;
    float a = e10.dot(e10);
    float b = e10.dot(e20);
    float c = e20.dot(e20);
    float ac_bb=(a*c)-(b*b);
    Vector3f vp(point.x-pa.x, point.y-pa.y, point.z-pa.z);
    float d = vp.dot(e10);
    float e = vp.dot(e20);
    float x = (d*c)-(e*b);
    float y = (e*a)-(d*b);
    float z = x+y-ac_bb;
    return (( in(z)& ~(in(x)|in(y)) ) & 0x80000000);
}

bool getLowestRoot(float a, float b, float c, float maxR, float* root) {
    // Check if a solution exists
    float determinant = b*b - 4.0f*a*c;
    // If determinant is negative it means no solutions.
    if (determinant < 0.0f) return false;
    // calculate the two roots: (if determinant == 0 then
    // x1==x2 but let’s disregard that slight optimization)
    float sqrtD = sqrt(determinant);
    float r1 = (-b - sqrtD) / (2*a);
    float r2 = (-b + sqrtD) / (2*a);
    // Sort so x1 <= x2
    if (r1 > r2) {
        float temp = r2;
        r2 = r1;
        r1 = temp;
    }
    // Get lowest root:
    if (r1 > 0 && r1 < maxR) {
        *root = r1;
        return true;
    }
    // It is possible that we want x2 - this can happen
    // if x1 < 0
    if (r2 > 0 && r2 < maxR) {
        *root = r2;
        return true;
    }
    // No (valid) solutions
    return false;
}

void checkTriangle(CollisionPacket* colPackage,const Vector3f& p1,const Vector3f& p2,const Vector3f& p3)
{
    // Make the plane containing this triangle.
    PLANE trianglePlane(p1,p2,p3);
    // Is triangle front-facing to the velocity vector?
    // We only check front-facing triangles
    // (your choice of course)
    if (trianglePlane.isFrontFacingTo(colPackage->normalizedVelocity)) {
        // Get interval of plane intersection:
        double t0, t1;
        bool embeddedInPlane = false;
        // Calculate the signed distance from sphere
        // position to triangle plane
        double signedDistToTrianglePlane =
        trianglePlane.signedDistanceTo(colPackage->basePoint);
        // cache this as we’re going to use it a few times below:
        float normalDotVelocity =
        trianglePlane.normal.dot(colPackage->velocity);
        // if sphere is travelling parrallel to the plane:
        if (normalDotVelocity == 0.0f) {
            if (fabs(signedDistToTrianglePlane) >= 1.0f) {
                // Sphere is not embedded in plane.
                // No collision possible:
                return;
            }
            else {
                // sphere is embedded in plane.
                // It intersects in the whole range [0..1]
                embeddedInPlane = true;
                t0 = 0.0;
                t1 = 1.0;
            }
        }
        else {
            // N dot D is not 0. Calculate intersection interval:
            t0=(-1.0-signedDistToTrianglePlane)/normalDotVelocity;
            t1=( 1.0-signedDistToTrianglePlane)/normalDotVelocity;
            // Swap so t0 < t1
            if (t0 > t1) {
                double temp = t1;
                t1 = t0;
                t0 = temp;
            }
            // Check that at least one result is within range:
            if (t0 > 1.0f || t1 < 0.0f) {
                // Both t values are outside values [0,1]
                // No collision possible:
                return;
            }
            // Clamp to [0,1]
            if (t0 < 0.0) t0 = 0.0;
            if (t1 < 0.0) t1 = 0.0;
            if (t0 > 1.0) t0 = 1.0;
            if (t1 > 1.0) t1 = 1.0;
        }
        // OK, at this point we have two time values t0 and t1
        // between which the swept sphere intersects with the
        // triangle plane. If any collision is to occur it must
        // happen within this interval.
        Vector3f collisionPoint;
        bool foundCollison = false;
        float t = 1.0;
        // First we check for the easy case - collision inside
        // the triangle. If this happens it must be at time t0
        // as this is when the sphere rests on the front side
        // of the triangle plane. Note, this can only happen if
        // the sphere is not embedded in the triangle plane.
        if (!embeddedInPlane) {
        Vector3f planeIntersectionPoint = (colPackage->basePoint-trianglePlane.normal) + t0*colPackage->velocity;
            if (checkPointInTriangle(planeIntersectionPoint,p1,p2,p3))
            {
                foundCollison = true;
                t = t0;
                collisionPoint = planeIntersectionPoint;
            }
        }
        // if we haven’t found a collision already we’ll have to
        // sweep sphere against points and edges of the triangle.
        // Note: A collision inside the triangle (the check above)
        // will always happen before a vertex or edge collision!
        // This is why we can skip the swept test if the above
        // gives a collision!
        if (foundCollison == false) {
        // some commonly used terms:
        Vector3f velocity = colPackage->velocity;
        Vector3f base = colPackage->basePoint;
        float velocitySquaredLength = velocity.squaredLength();
        float a,b,c; // Params for equation
        float newT;
        // For each vertex or edge a quadratic equation have to
        // be solved. We parameterize this equation as
        // a*t^2 + b*t + c = 0 and below we calculate the
        // parameters a,b and c for each test.
        // Check against points:
        a = velocitySquaredLength;
        // P1
        b = 2.0*(velocity.dot(base-p1));
        c = (p1-base).squaredLength() - 1.0;
        if (getLowestRoot(a,b,c, t, &newT)) {
            t = newT;
            foundCollison = true;
            collisionPoint = p1;
        }
        // P2
        b = 2.0*(velocity.dot(base-p2));
        c = (p2-base).squaredLength() - 1.0;
        if (getLowestRoot(a,b,c, t, &newT)) {
            t = newT;
            foundCollison = true;
            collisionPoint = p2;
        }
        // P3
        b = 2.0*(velocity.dot(base-p3));
        c = (p3-base).squaredLength() - 1.0;
        if (getLowestRoot(a,b,c, t, &newT)) {
            t = newT;
            foundCollison = true;
            collisionPoint = p3;
        }
        // Check agains edges:
        // p1 -> p2:
        Vector3f edge = p2-p1;
        Vector3f baseToVertex = p1 - base;
        float edgeSquaredLength = edge.squaredLength();
        float edgeDotVelocity = edge.dot(velocity);
        float edgeDotBaseToVertex = edge.dot(baseToVertex);
        // Calculate parameters for equation
        a = edgeSquaredLength*-velocitySquaredLength + edgeDotVelocity*edgeDotVelocity;
        b = edgeSquaredLength*(2*velocity.dot(baseToVertex)) - 2.0*edgeDotVelocity*edgeDotBaseToVertex;
        c = edgeSquaredLength*(1-baseToVertex.squaredLength()) + edgeDotBaseToVertex*edgeDotBaseToVertex;
        // Does the swept sphere collide against infinite edge?
        if (getLowestRoot(a,b,c, t, &newT)) {
            // Check if intersection is within line segment:
            float f=(edgeDotVelocity*newT-edgeDotBaseToVertex)/edgeSquaredLength;
            if (f >= 0.0 && f <= 1.0) {
                // intersection took place within segment.
                t = newT;
                foundCollison = true;
                collisionPoint = p1 + f*edge;
            }
        }
        // p2 -> p3:
        edge = p3-p2;
        baseToVertex = p2 - base;
        edgeSquaredLength = edge.squaredLength();
        edgeDotVelocity = edge.dot(velocity);
        edgeDotBaseToVertex = edge.dot(baseToVertex);
        a = edgeSquaredLength*-velocitySquaredLength + edgeDotVelocity*edgeDotVelocity;
        b = edgeSquaredLength*(2*velocity.dot(baseToVertex)) - 2.0*edgeDotVelocity*edgeDotBaseToVertex;
        c = edgeSquaredLength*(1-baseToVertex.squaredLength()) + edgeDotBaseToVertex*edgeDotBaseToVertex;
        if (getLowestRoot(a,b,c, t, &newT)) {
            float f=(edgeDotVelocity*newT-edgeDotBaseToVertex)/edgeSquaredLength;
            if (f >= 0.0 && f <= 1.0) {
                t = newT;
                foundCollison = true;
                collisionPoint = p2 + f*edge;
            }
        }
        // p3 -> p1:
        edge = p1-p3;
        baseToVertex = p3 - base;
        edgeSquaredLength = edge.squaredLength();
        edgeDotVelocity = edge.dot(velocity);
        42edgeDotBaseToVertex = edge.dot(baseToVertex);
        a = edgeSquaredLength*-velocitySquaredLength + edgeDotVelocity*edgeDotVelocity;
        b = edgeSquaredLength*(2*velocity.dot(baseToVertex)) - 2.0*edgeDotVelocity*edgeDotBaseToVertex;
        c = edgeSquaredLength*(1-baseToVertex.squaredLength()) + edgeDotBaseToVertex*edgeDotBaseToVertex;
        if (getLowestRoot(a,b,c, t, &newT)) {
            float f=(edgeDotVelocity*newT-edgeDotBaseToVertex)/edgeSquaredLength;
            if (f >= 0.0 && f <= 1.0) {
                t = newT;
                foundCollison = true;
                collisionPoint = p3 + f*edge;
            }
        }
        }
        // Set result:
        if (foundCollison == true) {
            // distance to collision: ’t’ is time of collision
            float distToCollision = t*colPackage->velocity.length();
            // Does this triangle qualify for the closest hit?
            // it does if it’s the first hit or the closest
            if (colPackage->foundCollision == false ||
                distToCollision < colPackage->nearestDistance) {
                // Collision information nessesary for sliding
                colPackage->nearestDistance = distToCollision;
                colPackage->intersectionPoint=collisionPoint;
                colPackage->foundCollision = true;
            }
        }
    } // if not backface
}

void CharacterEntity::collideAndSlide(const Vector3f& vel, const Vector3f& gravity)
{
    // Do collision detection:
    collisionPackage->R3Position = position;
    collisionPackage->R3Velocity = vel;
    // calculate position and velocity in eSpace
    Vector3f eSpacePosition = collisionPackage->R3Position/
    collisionPackage->eRadius;
    Vector3f eSpaceVelocity = collisionPackage->R3Velocity/
    collisionPackage->eRadius;
    // Iterate until we have our final position.
    collisionRecursionDepth = 0;
    Vector3f finalPosition = collideWithWorld(eSpacePosition,
    eSpaceVelocity);
    // Add gravity pull:
    // To remove gravity uncomment from here .....
    // Set the new R3 position (convert back from eSpace to R3
    collisionPackage->R3Position = finalPosition*collisionPackage->eRadius;
    collisionPackage->R3Velocity = gravity;
    eSpaceVelocity = gravity/collisionPackage->eRadius;
    collisionRecursionDepth = 0;
    finalPosition = collideWithWorld(finalPosition,eSpaceVelocity);
    // ... to here
    // Convert final result back to R3:
    finalPosition = finalPosition*collisionPackage->eRadius;
    // Move the entity (application specific function)
    MoveTo(finalPosition);
{

// Set this to match application scale..
const float unitsPerMeter = 100.0f;

Vector3f CharacterEntity::collideWithWorld(const Vector3f& pos, const Vector3f& vel)
{
    // All hard-coded distances in this function is
    // scaled to fit the setting above..
    float unitScale = unitsPerMeter / 100.0f;
    float veryCloseDistance = 0.005f * unitScale;
    // do we need to worry?
    if (collisionRecursionDepth>5)
        return pos;
    // Ok, we need to worry:
    collisionPackage->velocity = vel;
    collisionPackage->normalizedVelocity = vel;
    collisionPackage->normalizedVelocity.normalize();
    collisionPackage->basePoint = pos;
    collisionPackage->foundCollision = false;
    // Check for collision (calls the collision routines)
    // Application specific!!
    world->checkCollision(collisionPackage);
    // If no collision we just move along the velocity
    if (collisionPackage->foundCollision == false) {
        return pos + vel;
    }
    // *** Collision occured ***
    // The original destination point
    Vector3f destinationPoint = pos + vel;
    Vector3f newBasePoint = pos;
    // only update if we are not already very close
    // and if so we only move very close to intersection..not
    // to the exact spot.
    if (collisionPackage->nearestDistance>=veryCloseDistance)
    {
        Vector3f V = vel;
        V.SetLength(collisionPackage->nearestDistanceveryCloseDistance);
        newBasePoint = collisionPackage->basePoint + V;
        // Adjust polygon intersection point (so sliding
        // plane will be unaffected by the fact that we
        // move slightly less than collision tells us)
        V.normalize();
        collisionPackage->intersectionPoint -= veryCloseDistance * V;
    }
    // Determine the sliding plane
    Vector3f slidePlaneOrigin = collisionPackage->intersectionPoint;
    Vector3f slidePlaneNormal = newBasePoint - collisionPackage->intersectionPoint;
    slidePlaneNormal = normalize(slidePlaneNormal);
    PLANE slidingPlane(slidePlaneOrigin,slidePlaneNormal);
    // Again, sorry about formatting.. but look carefully ;)
    Vector3f newDestinationPoint = destinationPoint - slidingPlane.signedDistanceTo(destinationPoint)*slidePlaneNormal;
    // Generate the slide vector, which will become our new
    // velocity vector for the next iteration
    Vector3f newVelocityVector = newDestinationPoint - collisionPackage->intersectionPoint;
    // Recurse:
    // dont recurse if the new velocity is very small
    if (newVelocityVector.length() < veryCloseDistance) {
        return newBasePoint;
    }
    collisionRecursionDepth++;
    return collideWithWorld(newBasePoint,newVelocityVector);
}
