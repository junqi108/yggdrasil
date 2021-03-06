import os
from yggdrasil.examples.tests import ExampleTstBase
try:
    import roadrunner
    rr_ver = tuple([int(x) for x in roadrunner.__version__.split('.')])
except ImportError:
    rr_ver = (0, 0, 0)


class TestExampleSBML1(ExampleTstBase):
    r"""Base class for SBML example tests."""

    example_name = 'sbml1'

    @property
    def input_files(self):  # pragma: no cover
        r"""Input file."""
        return [os.path.join(self.yamldir, 'Input', 'input.txt')]

    @property
    def expected_output_files(self):
        r"""Expected output file."""
        if rr_ver < (2, 0, 0):
            return [os.path.join(self.yamldir, 'Output', 'expected.txt')]
        else:
            return [os.path.join(self.yamldir, 'Output', 'expected-2.0.0.txt')]
        
    @property
    def output_files(self):
        r"""Output file."""
        return [os.path.join(self.yamldir, 'output.txt')]
    

class TestExampleSBML2(TestExampleSBML1):
    r"""Test for SBML2 example."""
    
    example_name = 'sbml2'

    @property
    def expected_output_files(self):
        r"""Expected output file."""
        if rr_ver < (2, 0, 0):
            return [os.path.join(self.yamldir, 'Output', 'expected%d.txt')
                    % i for i in range(2)]
        else:
            return [os.path.join(self.yamldir, 'Output', 'expected%d-2.0.0.txt')
                    % i for i in range(2)]
        
    @property
    def output_files(self):
        r"""Output file."""
        return [os.path.join(self.yamldir, 'output%d.txt')
                % i for i in range(2)]


class TestExampleSBML3(TestExampleSBML1):
    r"""Test for SBML3 example."""
    
    example_name = 'sbml3'
