#version 400

in Fragment {
  vec4 color;
  vec2 mapping;
} fragment;

layout(location = 0) out vec4 fragmentColor;

void main() {
  float r = dot(fragment.mapping, fragment.mapping);
  if (r > 1.0) discard;

  // Smooth circular falloff (soft blob edges)
  float alpha = pow(1.0 - r, 2.5); // soft edge, not harsh glow

  // You can multiply by color.a or intensity if you want modulation
  fragmentColor = vec4(fragment.color.rgb * alpha, alpha);
}
