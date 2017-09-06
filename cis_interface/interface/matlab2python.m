function x_py = matlab2python(x_ml)
  if isscalar(x_ml)
    if isa(x_ml, 'float')
      x_py = py.float(x_ml);
    elseif isa(x_ml, 'integer')
      x_py = py.int(x_ml);
    elseif isa(x_ml, 'string')
      x_py = py.str(x_ml);
    elseif isa(x_ml, 'logical')
      x_py = py.bool(x_ml);
    elseif isa(x_ml, 'struct')
      x_py = py.dict(x_ml);
    else
      disp('Could not convert scalar matlab type to python type');
      disp(x_ml);
      x_py = x_ml;
    end
  elseif isvector(x_ml)
    x_py = py.numpy.array(x_ml);
  elseif ismatrix(x_ml)
    data_size = size(x_ml);
    transpose = x_ml';
    x_py = py.numpy.reshape(transpose(:)', data_size);
  else
    disp('Could not convert matlab type to python type');
    disp(x_ml);
    x_py = x_ml;
  end
end