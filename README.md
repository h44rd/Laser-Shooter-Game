# Laser Shooter

Demo: https://youtu.be/eeWiamEIh30

Basic controls are the same as the ones mentioned in the problem statement.
Additional Controls / Features:
1. The speed of the laser can be increased and decreased with c and v keys respectively.
2. The faster the laser, the less rules of the game it tends to follow.
3. Pressing t will toggle between mouse and Keyboard controls.
    ->Dragging the buckets and cannon with mouse will be possible only if
      mouse controls are turn off.
    ->Turning off mouse controls will result in the cannon NOT following the
      mouse cursor.
4. Pressing r will toggle between reflecting and refracting surfaces.
    ->Turning on refraction will result in the laser just passing through
      the surface.
    ->Refracting angle will be the same as initial collision angle.
5. Multiple bricks can be destroyed with one laser bullet.
6. The speed of the bullet is reflected in the speed with which the cannon is rotating.
7. Cannon takes one second to reload.
8. No penalty is given if a green brick falls in red bucket or vice versa.
9. Game ends once a black brick falls into a bucket.

After installing the dependencies, run using:
```
make

./sample2D
```
