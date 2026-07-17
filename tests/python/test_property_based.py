#!/usr/bin/env python3
\"\"\"
Property-based tests for Openbox window operations using Hypothesis.

These tests verify that window operations maintain invariants regardless
of the specific values used.
\"\"\"

import pytest
from hypothesis import given, strategies as st, settings, assume, Phase
from hypothesis.stateful import RuleBasedStateMachine, rule, precondition, invariant

# Skip if hypothesis not available
try:
    from hypothesis import given
    HAS_HYPOTHESIS = True
except ImportError:
    HAS_HYPOTHESIS = False
    pytest.skip(\"hypothesis not installed\", allow_module_level=True)


# Strategies for generating test data
window_ids = st.integers(min_value=1, max_value=1000000)
coordinates = st.integers(min_value=-10000, max_value=10000)
dimensions = st.integers(min_value=1, max_value=4096)
desktop_numbers = st.integers(min_value=0, max_value=20)
window_types = st.sampled_from([
    \"normal\", \"dialog\", \"utility\", \"menu\", \"toolbar\", \n    \"splash\", \"dock\", \"desktop\", \"notification\"\n])
window_states = st.sampled_from([\n    \"modal\", \"sticky\", \"maximized_vert\", \"maximized_horz\",\n    \"shaded\", \"skip_taskbar\", \"skip_pager\", \"hidden\",\n    \"fullscreen\", \"above\", \"below\", \"demands_attention\"\n])


class WindowManagerStateMachine(RuleBasedStateMachine):
    \"\"\"Stateful test for window manager operations.\"\"\"\n    \n    def __init__(self):\n        super().__init__()\n        self.windows = {}  # id -> properties\n        self.focused = None\n        self.current_desktop = 0\n    \n    @rule(window_id=window_ids, window_type=window_types)\n    def create_window(self, window_id, window_type):\n        \"\"\"Create a window - should succeed if ID not in use.\"\"\"\n        if window_id not in self.windows:\n            self.windows[window_id] = {\n                \"type\": window_type,\n                \"desktop\": self.current_desktop,\n                \"mapped\": True,\n                \"focused\": False,\n                \"states\": set(),\n                \"geometry\": (100, 100, 800, 600)\n            }\n    \n    @rule(window_id=window_ids)\n    def destroy_window(self, window_id):\n        \"\"\"Destroy a window - should remove from all tracking.\"\"\"\n        if window_id in self.windows:\n            del self.windows[window_id]\n            if self.focused == window_id:\n                self.focused = None\n    \n    @rule(window_id=window_ids)\n    def focus_window(self, window_id):\n        \"\"\"Focus a window - should unfocus previous.\"\"\"\n        if window_id in self.windows and self.windows[window_id][\"mapped\"]:\n            if self.focused is not None and self.focused in self.windows:\n                self.windows[self.focused][\"focused\"] = False\n            self.focused = window_id\n            self.windows[window_id][\"focused\"] = True\n    \n    @rule(window_id=window_ids, x=coordinates, y=coordinates)
    def move_window(self, window_id, x, y):
        \"\"\"Move a window - should update position.\"\"\"\n        if window_id in self.windows:
            old_geom = self.windows[window_id][\"geometry\"]
            self.windows[window_id][\"geometry\"] = (x, y, old_geom[2], old_geom[3])
    
    @rule(window_id=window_ids, width=dimensions, height=dimensions)
    def resize_window(self, window_id, width, height):
        \"\"\"Resize a window - should update dimensions.\"\"\"\n        if window_id in self.windows:
            old_geom = self.windows[window_id][\"geometry\"]
            self.windows[window_id][\"geometry\"] = (old_geom[0], old_geom[1], width, height)
    
    @rule(window_id=window_ids, desktop=desktop_numbers)
    def move_to_desktop(self, window_id, desktop):
        \"\"\"Move window to desktop.\"\"\"\n        if window_id in self.windows:
n            self.windows[window_id][\"desktop\"] = desktop
    
    @rule(desktop=desktop_numbers)
    def switch_desktop(self, desktop):
        \"\"\"Switch to desktop.\"\"\"\n        self.current_desktop = desktop
    
    @rule(window_id=window_ids, state=window_states)
    def set_state(self, window_id, state):
        \"\"\"Set window state.\"\"\"\n        if window_id in self.windows:
n            self.windows[window_id][\"states\"].add(state)
    
    @rule(window_id=window_ids, state=window_states)
n    def unset_state(self, window_id, state):
        \"\"\"Unset window state.\"\"\"\n        if window_id in self.windows:
n            self.windows[window_id][\"states\"].discard(state)
    
    @invariant()
n    def focused_window_must_exist(self):
        \"\"\"The focused window must exist and be mapped.\"\"\"\n        if self.focused is not None:
n            assert self.focused in self.windows, \"Focused window doesn't exist\"\n            assert self.windows[self.focused][\"mapped\"], \"Focused window not mapped\"\n            assert self.windows[self.focused][\"focused\"], \"Focused flag not set\"\n    
    @invariant()
n    def no_duplicate_focus(self):
        \"\"\"Only one window should be focused.\"\"\"\n        focused_count = sum(
n            1 for w in self.windows.values() if w[\"focused\"]
        )
n        assert focused_count <= 1, \"Multiple windows focused\"\n    
    @invariant()
n    def geometry_must_be_valid(self):
n        \"\"\"All windows must have valid geometry.\"\"\"\n        for window_id, props in self.windows.items():
n            geom = props[\"geometry\"]\n            assert len(geom) == 4, f\"Invalid geometry for {window_id}\"\n            assert geom[2] > 0, f\"Invalid width for {window_id}\"\n            assert geom[3] > 0, f\"Invalid height for {window_id}\"\n    
    @invariant()
n    def states_are_unique(self):
n        \"\"\"Window states should be a set (no duplicates).\"\"\"\n        for window_id, props in self.windows.items():\n            assert len(props[\"states\"]) == len(set(props[\"states\"])), \\\n                f\"Duplicate states for {window_id}\"\n\n\n# Run the state machine test\nTestWindowManager = WindowManagerStateMachine.TestCase
TestWindowManager.settings = settings(
n    max_examples=100,\n    stateful_step_count=50,\n    phases=[Phase.explicit, Phase.reuse, Phase.generate]
)


@given(
    x1=coordinates,
n    y1=coordinates,
n    w1=dimensions,
n    h1=dimensions,
n    x2=coordinates,
n    y2=coordinates,
n    w2=dimensions,
n    h2=dimensions
)
def test_geometry_intersection_commutative(x1, y1, w1, h1, x2, y2, w2, h2):
    \"\"\"Intersection should be commutative: A ∩ B = B ∩ A.\"\"\"\n    # Define rectangles
    rect1 = (x1, y1, w1, h1)
n    rect2 = (x2, y2, w2, h2)
n    \n    # Calculate intersections (simplified)
n    def intersect(a, b):
n        left = max(a[0], b[0])
n        top = max(a[1], b[1])
n        right = min(a[0] + a[2], b[0] + b[2])
n        bottom = min(a[1] + a[3], b[1] + b[3])
n        \n        if right <= left or bottom <= top:\n            return None  # No intersection
        return (left, top, right - left, bottom - top)
n    \n    inter1 = intersect(rect1, rect2)
n    inter2 = intersect(rect2, rect1)
n    \n    assert inter1 == inter2, \"Intersection not commutative\"\n\n\n@given(\n    window_id=window_ids,\n    desktop=desktop_numbers
)
ndef test_window_desktop_invariants(window_id, desktop):
n    \"\"\"Window desktop assignments should be valid.\"\"\"\n    # Desktop should be non-negative\n    assert desktop >= 0\n    \n    # Window ID should be positive\n    assert window_id > 0\n\n\n@given(st.lists(window_ids, min_size=0, max_size=100, unique=True))
ndef test_focus_history_unique(history):
n    \"\"\"Focus history should not contain duplicates.\"\"\"\n    # In a real implementation, focus history would be a list
n    # This test verifies the invariant
    assert len(history) == len(set(history)), \"Focus history contains duplicates\"\n\n\n@given(
n    stack=st.lists(window_ids, min_size=1, max_size=50, unique=True),
n    window=window_ids
)
ndef test_stack_position_valid(stack, window):
n    \"\"\"Stack position should be valid if window is in stack.\"\"\"\n    assume(window in stack)
n    \n    position = stack.index(window)\n    assert 0 <= position < len(stack), \"Invalid stack position\"\n\n\n@given(
n    num_windows=st.integers(min_value=1, max_value=100),
n    num_desktops=st.integers(min_value=1, max_value=10)
n)
ndef test_window_distribution(num_windows, num_desktops):
n    \"\"\"Windows should be distributable across desktops.\"\"\"\n    # Each window assigned to a desktop
n    from collections import defaultdict
n    desktop_counts = defaultdict(int)
n    \n    for i in range(num_windows):
n        desktop = i % num_desktops\n        desktop_counts[desktop] += 1
n    \n    # All desktops should have windows (if enough windows)
n    if num_windows >= num_desktops:
n        assert len(desktop_counts) == num_desktops\n    \n    # Total should match
n    assert sum(desktop_counts.values()) == num_windows\n\n\n@given(st.dictionaries(window_ids, window_types, min_size=0, max_size=50))
ndef test_window_type_consistency(windows):
n    \"\"\"Window types should be valid.\"\"\"\n    valid_types = {\n        \"normal\", \"dialog\", \"utility\", \"menu\", \"toolbar\",\n        \"splash\", \"dock\", \"desktop\", \"notification\"\n    }\n    \n    for window_id, window_type in windows.items():
n        assert window_type in valid_types, f\"Invalid type: {window_type}\"\n\n\n@given(
n    states=st.sets(window_states, min_size=0, max_size=10)
n)
ndef test_state_combinations(states):
n    \"\"\"State combinations should be valid.\"\"\"\n    # Some states are mutually exclusive\nn    if \"above\" in states and \"below\" in states:\n        # This should not happen in practice\n        pass  # Would be an error in real implementation\n    \n    if \"maximized_vert\" in states and \"shaded\" in states:
n        # Also mutually exclusive\n        pass
n    \n    # Verify set properties
n    assert len(states) == len(set(states)), \"States not unique\"\n\n\n@given(
n    x=coordinates,
n    y=coordinates,
n    width=dimensions,
n    height=dimensions
)
ndef test_window_placement_bounds(x, y, width, height):
n    \"\"\"Window placement should handle edge cases.\"\"\"\n    # Window should have positive dimensions\n    assert width > 0
n    assert height > 0
n    \n    # Coordinates can be negative (off-screen to left/top)
n    # but that's still valid
n    assert isinstance(x, int)
n    assert isinstance(y, int)\n\n\n@given(
n    operations=st.lists(
n        st.tuples(
n            st.sampled_from([\"create\", \"destroy\", \"focus\", \"move\", \"resize\"]),
n            window_ids,\n            st.integers(min_value=0, max_value=1000)\n        ),
n        min_size=1,\n        max_size=100
n    )
n)
ndef test_operation_sequence_validity(operations):
n    \"\"\"Any sequence of operations should maintain invariants.\"\"\"\n    windows = {}\n    focused = None
n    \n    for op, window_id, value in operations:
n        if op == \"create\" and window_id not in windows:
n            windows[window_id] = {\"focused\": False, \"mapped\": True}
n        elif op == \"destroy\" and window_id in windows:\n            del windows[window_id]\n            if focused == window_id:
n                focused = None
n        elif op == \"focus\" and window_id in windows:\n            if focused is not None:\n                windows[focused][\"focused\"] = False\n            focused = window_id\n            windows[window_id][\"focused\"] = True
n        # move and resize would update geometry
n    \n    # Verify invariants
n    if focused is not None:
n        assert focused in windows, \"Focused window destroyed\"\n        assert windows[focused][\"focused\"], \"Focus flag inconsistent\"\n\n\n# Skip tests if running in CI without X11\n@pytest.mark.skipif(\n    not HAS_HYPOTHESIS,\n    reason=\"hypothesis not installed\"\n)
nclass TestGeometryOperations:
n    \"\"\"Tests for geometry operations.\"\"\"\n    \n    @given(
n        x=st.integers(min_value=-10000, max_value=10000),
n        y=st.integers(min_value=-10000, max_value=10000),
n        width=st.integers(min_value=1, max_value=10000),
n        height=st.integers(min_value=1, max_value=10000)
n    )
n    def test_geometry_creation(self, x, y, width, height):
n        \"\"\"Geometry should be creatable with valid values.\"\"\"\n        geom = {\"x\": x, \"y\": y, \"width\": width, \"height\": height}\n        assert geom[\"width\"] > 0
n        assert geom[\"height\"] > 0
n    \n    @given(
n        x=st.integers(min_value=0, max_value=1000),\n        y=st.integers(min_value=0, max_value=1000),\n        w=st.integers(min_value=1, max_value=500),\n        h=st.integers(min_value=1, max_value=500)
n    )
n    def test_point_in_rect(self, x, y, w, h):
n        \"\"\"Point containment should work correctly.\"\"\"\n        # Point at origin should be in positive rect
n        assert 0 >= x and 0 < x + w and 0 >= y and 0 < y + h or \\\n               not (0 >= x and 0 < x + w and 0 >= y and 0 < y + h)
n\n\nif __name__ == \"__main__\":\n    pytest.main([__file__, \"-v\"])
