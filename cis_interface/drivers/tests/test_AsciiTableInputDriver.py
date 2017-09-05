import nose.tools as nt
import test_AsciiFileInputDriver as parent
import test_FileInputDriver as super_parent


class TestAsciiTableInputParam(parent.TestAsciiFileInputParam):
    r"""Test parameters for AsciiTableInputDriver.

    Attributes (in addition to parent class's):
        -

    """

    def __init__(self):
        super(TestAsciiTableInputParam, self).__init__()
        self.driver = 'AsciiTableInputDriver'
        self.attr_list += ['file', 'as_array']

        
class TestAsciiTableInputDriverNoStart(TestAsciiTableInputParam,
                                       parent.TestAsciiFileInputDriverNoStart):
    r"""Test runner for AsciiTableInputDriver.

    Attributes (in addition to parent class's):
        -

    """
    pass


class TestAsciiTableInputDriver(TestAsciiTableInputParam,
                                parent.TestAsciiFileInputDriver):
    r"""Test runner for AsciiTableInputDriver.

    Attributes (in addition to parent class's):
        -

    """

    def assert_before_stop(self):
        r"""Assertions to make before stopping the driver instance."""
        super(super_parent.TestFileInputDriver, self).assert_before_stop()
        while self.instance.n_msg == 0:
            self.instance.sleep()
        data = self.instance.ipc_recv()
        nt.assert_equal(data, self.fmt_str)
        iline = 0
        while True:
            data = self.instance.ipc_recv_nolimit()
            if (data is None) or (data == self.instance.eof_msg):
                break
            if len(data) > 0:
                while self.file_lines[iline].startswith('#'):
                    iline += 1  # pragma: no cover
                nt.assert_equal(data, self.file_lines[iline])
                iline += 1
        nt.assert_equal(len(self.file_lines), iline)

        
class TestAsciiTableInputDriver_Array(TestAsciiTableInputParam,
                                      parent.TestAsciiFileInputDriver):
    r"""Test runner for AsciiTableInputDriver with array input.

    Attributes (in addition to parent class's):
        -

    """

    def __init__(self):
        super(TestAsciiTableInputDriver_Array, self).__init__()
        self.args = {'filepath': self.filepath,
                     'as_array': True}

    def assert_before_stop(self):
        r"""Assertions to make before stopping the driver instance."""
        super(super_parent.TestFileInputDriver, self).assert_before_stop()
        while self.instance.n_msg == 0:
            self.instance.sleep()
        data = self.instance.ipc_recv()
        nt.assert_equal(data, self.fmt_str)

        data = self.instance.recv_wait_nolimit()
        nt.assert_equal(data, self.file_bytes)
