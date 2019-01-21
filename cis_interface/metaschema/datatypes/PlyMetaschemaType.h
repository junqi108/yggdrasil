#ifndef PLY_METASCHEMA_TYPE_H_
#define PLY_METASCHEMA_TYPE_H_

#include "../../tools.h"
#include "MetaschemaType.h"
#include "PlyDict.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"


class PlyMetaschemaType : public MetaschemaType {
public:
  PlyMetaschemaType() : MetaschemaType("ply") {}
  PlyMetaschemaType(const rapidjson::Value &type_doc) : MetaschemaType(type_doc) {}
  PlyMetaschemaType* copy() { return (new PlyMetaschemaType()); }
  virtual size_t nargs_exp() {
    return 1;
  }

  // Encoding
  bool encode_data(rapidjson::Writer<rapidjson::StringBuffer> *writer,
		   size_t *nargs, va_list_t &ap) {
    // Get argument
    ply_t p = va_arg(ap.va, ply_t);
    (*nargs)--;
    // Allocate buffer
    int buf_size = 1000;
    char *buf = (char*)malloc(buf_size);
    int msg_len = 0, ilen = 0;
    // Format header
    char header_format[500] = "ply\n"
      "format ascii 1.0\n"
      "comment author ygg_auto\n"
      "comment File generated by yggdrasil\n"
      "element vertex %d\n"
      "property float x\n"
      "property float y\n"
      "property float z\n";
    if (p.vertex_colors != NULL) {
      char header_format_colors[100] = "property uchar diffuse_red\n"
	"property uchar diffuse_green\n"
	"property uchar diffuse_blue\n";
      strcat(header_format, header_format_colors);
    }
    char header_format2[100] = "element face %d\n"
      "property list uchar int vertex_index\n"
      "end_header\n";
    strcat(header_format, header_format2);
    while (true) {
      ilen = snprintf(buf, buf_size, header_format, p.nvert, p.nface);
      if (ilen < 0) {
	ygglog_error("PlyMetaschemaType::encode_data: Error formatting header.");
	return false;
      } else if (ilen >= buf_size) {
	buf_size = buf_size + ilen + 1;
	buf = (char*)realloc(buf, buf_size);
      } else {
	break;
      }
    }
    msg_len = msg_len + ilen;
    // Add vertex information
    int i, j;
    for (i = 0; i < p.nvert; i++) {
      while (true) {
	if (p.vertex_colors != NULL) {
	  ilen = snprintf(buf + msg_len, buf_size - msg_len, "%f %f %f %d %d %d\n",
			  p.vertices[i][0], p.vertices[i][1], p.vertices[i][2],
			  p.vertex_colors[i][0], p.vertex_colors[i][1], p.vertex_colors[i][2]);
	} else {
	  ilen = snprintf(buf + msg_len, buf_size - msg_len, "%f %f %f\n",
			  p.vertices[i][0], p.vertices[i][1], p.vertices[i][2]);
	}
	if (ilen < 0) {
	  ygglog_error("PlyMetaschemaType::encode_data: Error formatting vertex %d.", i);
	  return false;
	} else if (ilen >= (buf_size - msg_len)) {
	  buf_size = buf_size + ilen + 1;
	  buf = (char*)realloc(buf, buf_size);
	} else {
	  break;
	}
      }
      msg_len = msg_len + ilen;
    }
    // Add face information
    for (i = 0; i < p.nface; i++) {
      while (true) {
	ilen = snprintf(buf + msg_len, buf_size - msg_len, "%d", p.nvert_in_face[i]);
	if (ilen < 0) {
	  ygglog_error("PlyMetaschemaType::encode_data: Error formatting number of verts for face %d.", i);
	  return false;
	} else if (ilen >= (buf_size - msg_len)) {
	  buf_size = buf_size + ilen + 1;
	  buf = (char*)realloc(buf, buf_size);
	} else {
	  break;
	}
      }
      msg_len = msg_len + ilen;
      for (j = 0; j < p.nvert_in_face[i]; j++) {
	while (true) {
	  ilen = snprintf(buf + msg_len, buf_size - msg_len, " %d", p.faces[i][j]);
	  if (ilen < 0) {
	    ygglog_error("PlyMetaschemaType::encode_data: Error formatting element %d of face %d.", j, i);
	    return false;
	  } else if (ilen >= (buf_size - msg_len)) {
	    buf_size = buf_size + ilen + 1;
	    buf = (char*)realloc(buf, buf_size);
	  } else {
	    break;
	  }
	}
	msg_len = msg_len + ilen;
      }
      while (true) {
	ilen = snprintf(buf + msg_len, buf_size - msg_len, "\n");
	if (ilen < 0) {
	  ygglog_error("PlyMetaschemaType::encode_data: Error formatting newline for face %d.", i);
	  return false;
	} else if (ilen >= (buf_size - msg_len)) {
	  buf_size = buf_size + ilen + 1;
	  buf = (char*)realloc(buf, buf_size);
	} else {
	  break;
	}
      }
      msg_len = msg_len + ilen;
    }
    buf[msg_len] = '\0';
    writer->String(buf, msg_len);
    return true;
  }

  // Decoded
  bool decode_data(rapidjson::Value &data, const int allow_realloc,
		   size_t *nargs, va_list_t &ap) {
    if (not data.IsString())
      ygglog_throw_error("PlyMetaschemaType::decode_data: Data is not a string.");
    // Get input data
    const char *buf = data.GetString();
    // size_t buf_siz = data.GetStringLength();
    // Get output argument
    ply_t *p;
    ply_t **pp;
    if (allow_realloc) {
      pp = va_arg(ap.va, ply_t**);
      p = (ply_t*)realloc(*pp, sizeof(ply_t));
      if (p == NULL)
	ygglog_throw_error("PlyMetaschemaType::decode_data: could not realloc pointer.");
      *pp = p;
      *p = init_ply();
    } else {
      p = va_arg(ap.va, ply_t*);
    }
    (*nargs)--;
    // Process buffer
    int out = 1;
    int do_colors = 0;
    int n_sub_matches;
    size_t *sind = NULL;
    size_t *eind = NULL;
    size_t *sind_body = NULL;
    size_t *eind_body = NULL;
    size_t value_size;
    char value[100];
    size_t begin_body = 0;
    int nlines = 0;
    int i, j;
    int nvert = 0, nface = 0;
    int line_no;
    size_t line_size;
    char iline[100];
    // Get info from the header.
    // Number of vertices
    if (out > 0) {
      n_sub_matches = find_matches("element vertex ([[:digit:]]+)\n", buf, &sind, &eind);
      if (n_sub_matches < 2) {
	ygglog_error("PlyMetaschemaType::decode_data: Could not locate number of vertices in ply header.");
	out = -1;
      }
      value_size = eind[1] - sind[1];
      memcpy(value, buf + sind[1], value_size);
      value[value_size] = '\0';
      nvert = atoi(value);
    }
    // Number of faces
    if (out > 0) {
      n_sub_matches = find_matches("element face ([[:digit:]]+)\n", buf, &sind, &eind);
      if (n_sub_matches < 2) {
	ygglog_error("PlyMetaschemaType::decode_data: Could not locate number of faces in ply header.");
	out = -1;
      }
      value_size = eind[1] - sind[1];
      memcpy(value, buf + sind[1], value_size);
      value[value_size] = '\0';
      nface = atoi(value);
    }
    // Color
    if (out > 0) {
      n_sub_matches = find_matches("green", buf, &sind, &eind);
      if (n_sub_matches != 0) {
	do_colors = 1;
      }
    }
    // End of header
    if (out > 0) {
      n_sub_matches = find_matches("end_header\n", buf, &sind, &eind);
      if (n_sub_matches < 1) {
	ygglog_error("PlyMetaschemaType::decode_data: Could not locate end of header.");
	out = -1;
      } else {
	begin_body = eind[0];
      }
    }
    // Locate lines
    if (out > 0) {
      int nlines_expected = nvert + nface;
      nlines = 0;
      sind_body = (size_t*)realloc(sind_body, (nlines_expected+1)*sizeof(size_t));
      eind_body = (size_t*)realloc(eind_body, (nlines_expected+1)*sizeof(size_t));
      size_t cur_pos = begin_body;
      while (1) {
	n_sub_matches = find_matches("([^\n]*)\n", buf + cur_pos, &sind, &eind);
	if (n_sub_matches < 2) {
	  // Check for line not terminated with newline
	  n_sub_matches = find_matches("([^\n]*)", buf + cur_pos, &sind, &eind);
	  if ((n_sub_matches < 2) || (sind == eind)) {
	    break;
	  }
	}
	if (nlines > nlines_expected) {
	  break;
	  // nlines_expected = nlines_expected + 50;
	  // sind_body = (size_t*)realloc(sind_body, (nlines_expected+1)*sizeof(size_t));
	  // eind_body = (size_t*)realloc(eind_body, (nlines_expected+1)*sizeof(size_t));
	}
	sind_body[nlines] = cur_pos + sind[1];
	eind_body[nlines] = cur_pos + eind[1];
	nlines++;
	cur_pos = cur_pos + eind[0];
      }
      if ((nvert + nface) > nlines) {
	ygglog_error("PlyMetaschemaType::decode_data: Not enough lines (%d) for %d vertices "
		     "and %d faces.",
		     nlines, nvert, nface);
	out = -1;
      }
    }
    // Allocate
    if (out > 0) {
      int ret = alloc_ply(p, nvert, nface, do_colors);
      if (ret < 0) {
	ygglog_error("PlyMetaschemaType::decode_data: Error allocating ply structure.");
	out = -1;
      }
    }
    // Get vertices
    if (out > 0) {
      int nexpected = 3;
      if (do_colors) {
	nexpected = 6;
      }
      char vert_re[80] = "([^ ]+) ([^ ]+) ([^ ]+)";
      // Parse each line
      for (i = 0; i < p->nvert; i++) {
	line_no = i;
	line_size = eind_body[line_no] - sind_body[line_no];
	memcpy(iline, buf + sind_body[line_no], line_size);
	iline[line_size] = '\0';
	n_sub_matches = find_matches(vert_re, iline, &sind, &eind);
	if (n_sub_matches != nexpected + 1) {
	  ygglog_error("PlyMetaschemaType::decode_data: Vertex should contain %d entries. "
		       "%d were found", nexpected, n_sub_matches - 1);
	  out = -1;
	  break;
	} else {
	  for (j = 0; j < 3; j++) {
	    p->vertices[i][j] = (float)atof(iline + sind[j + 1]);
	  }
	  if (do_colors) {
	    for (j = 0; j < 3; j++) {
	      p->vertex_colors[i][j] = atoi(iline + sind[j + 4]);
	    }
	  }
	}
      }
    }
    // Get faces
    if (out > 0) {
      int nexpected = 0;
      // Parse each line
      for (i = 0; i < p->nface; i++) {
	line_no = i + p->nvert;
	line_size = eind_body[line_no] - sind_body[line_no];
	memcpy(iline, buf + sind_body[line_no], line_size);
	iline[line_size] = '\0';
	nexpected = atoi(iline);
	p->nvert_in_face[i] = nexpected;
	char face_re[80] = "([^ ]+)";
	for (j = 0; j < nexpected; j++) {
	  strcat(face_re, " ([^ ]+)");
	}
	n_sub_matches = find_matches(face_re, iline, &sind, &eind);
	if (n_sub_matches < (nexpected + 2)) {
	  ygglog_error("PlyMetaschemaType::decode_data: Face should contain %d entries. "
		       "%d were found.", nexpected, n_sub_matches - 2);
	  out = -1;
	  break;
	} else {
	  int *iface = (int*)realloc(p->faces[i], nexpected*sizeof(int));
	  if (iface == NULL) {
	    ygglog_error("PlyMetaschemaType::decode_data: Could not allocate face %d.", i);
	    out = -1;
	    break;
	  } else {
	    p->faces[i] = iface;
	    for (j = 0; j < nexpected; j++) {
	      p->faces[i][j] = atoi(iline + sind[j + 2]);
	    }
	  }
	}
      }
    }
    // Return
    if (sind != NULL) free(sind); 
    if (eind != NULL) free(eind);
    if (sind_body != NULL) free(sind_body); 
    if (eind_body != NULL) free(eind_body);
    if (out < 0) {
      free_ply(p);
      return false;
    } else {
      return true;
    }
  }

};


#endif /*PLY_METASCHEMA_TYPE_H_*/
// Local Variables:
// mode: c++
// End:
