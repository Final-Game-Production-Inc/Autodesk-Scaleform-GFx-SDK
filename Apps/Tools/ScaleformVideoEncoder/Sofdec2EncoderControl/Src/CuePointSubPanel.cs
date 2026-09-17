using System;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace Sofdec2EncoderControl;

public class CuePointSubPanel : UserControl
{
	private IContainer components;

	private DataGridViewTextBoxColumn CuePointParam;

	private Button btnCueParamAdd;

	private DataGridView tblCuePointParams;

	private DataGridViewTextBoxColumn CuePointParamValue;

	private SplitContainer splitCuePoint;

	private DataGridView tblCuePoint;

	private DataGridViewTextBoxColumn CuePointName;

	private DataGridViewTextBoxColumn CuePointTime;

	private DataGridViewComboBoxColumn CuePointType;

	private Button btnCuePointFileSave;

	private TextBox tboxCuePointFile;

	private Button btnCuePointDelete;

	private Button btnCuePointFile;

	private Button btnCuePointAdd;

	private Button btnCueParamDelete;

	public CuePointSubPanel()
	{
		InitializeComponent();
	}

	public string GetCuePointTextPath()
	{
		string text = tboxCuePointFile.Text;
		if (!checkFilePath(text))
		{
			return string.Empty;
		}
		return text;
	}

	private static bool checkFilePath(string path)
	{
		char[] anyOf = new char[2] { '*', '?' };
		if (path.IndexOfAny(anyOf) >= 0)
		{
			return false;
		}
		if (!File.Exists(path))
		{
			return false;
		}
		return true;
	}

	private void btnCuePointFile_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Cue Point Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tboxCuePointFile.Text = openFileDialog.FileName;
		}
	}

	protected override void Dispose(bool disposing)
	{
		if (disposing && components != null)
		{
			components.Dispose();
		}
		base.Dispose(disposing);
	}

	private void InitializeComponent()
	{
		this.CuePointParam = new System.Windows.Forms.DataGridViewTextBoxColumn();
		this.btnCueParamAdd = new System.Windows.Forms.Button();
		this.tblCuePointParams = new System.Windows.Forms.DataGridView();
		this.CuePointParamValue = new System.Windows.Forms.DataGridViewTextBoxColumn();
		this.splitCuePoint = new System.Windows.Forms.SplitContainer();
		this.tblCuePoint = new System.Windows.Forms.DataGridView();
		this.CuePointName = new System.Windows.Forms.DataGridViewTextBoxColumn();
		this.CuePointTime = new System.Windows.Forms.DataGridViewTextBoxColumn();
		this.CuePointType = new System.Windows.Forms.DataGridViewComboBoxColumn();
		this.btnCuePointFileSave = new System.Windows.Forms.Button();
		this.tboxCuePointFile = new System.Windows.Forms.TextBox();
		this.btnCuePointDelete = new System.Windows.Forms.Button();
		this.btnCuePointFile = new System.Windows.Forms.Button();
		this.btnCuePointAdd = new System.Windows.Forms.Button();
		this.btnCueParamDelete = new System.Windows.Forms.Button();
		((System.ComponentModel.ISupportInitialize)this.tblCuePointParams).BeginInit();
		this.splitCuePoint.Panel1.SuspendLayout();
		this.splitCuePoint.Panel2.SuspendLayout();
		this.splitCuePoint.SuspendLayout();
		((System.ComponentModel.ISupportInitialize)this.tblCuePoint).BeginInit();
		base.SuspendLayout();
		this.CuePointParam.HeaderText = "Parameter Name";
		this.CuePointParam.Name = "CuePointParam";
		this.CuePointParam.ReadOnly = true;
		this.CuePointParam.Width = 111;
		this.btnCueParamAdd.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btnCueParamAdd.Location = new System.Drawing.Point(319, 3);
		this.btnCueParamAdd.Name = "btnCueParamAdd";
		this.btnCueParamAdd.Size = new System.Drawing.Size(26, 23);
		this.btnCueParamAdd.TabIndex = 10;
		this.btnCueParamAdd.Text = "+";
		this.btnCueParamAdd.UseVisualStyleBackColor = true;
		this.tblCuePointParams.AllowUserToAddRows = false;
		this.tblCuePointParams.AllowUserToDeleteRows = false;
		this.tblCuePointParams.AllowUserToResizeRows = false;
		this.tblCuePointParams.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tblCuePointParams.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.DisplayedCells;
		this.tblCuePointParams.ColumnHeadersHeight = 21;
		this.tblCuePointParams.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.DisableResizing;
		this.tblCuePointParams.Columns.AddRange(this.CuePointParam, this.CuePointParamValue);
		this.tblCuePointParams.Location = new System.Drawing.Point(0, 30);
		this.tblCuePointParams.Margin = new System.Windows.Forms.Padding(1);
		this.tblCuePointParams.Name = "tblCuePointParams";
		this.tblCuePointParams.ReadOnly = true;
		this.tblCuePointParams.Size = new System.Drawing.Size(375, 156);
		this.tblCuePointParams.TabIndex = 8;
		this.CuePointParamValue.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
		this.CuePointParamValue.FillWeight = 90f;
		this.CuePointParamValue.HeaderText = "Value";
		this.CuePointParamValue.Name = "CuePointParamValue";
		this.CuePointParamValue.ReadOnly = true;
		this.splitCuePoint.Dock = System.Windows.Forms.DockStyle.Fill;
		this.splitCuePoint.Location = new System.Drawing.Point(0, 0);
		this.splitCuePoint.Name = "splitCuePoint";
		this.splitCuePoint.Orientation = System.Windows.Forms.Orientation.Horizontal;
		this.splitCuePoint.Panel1.Controls.Add(this.tblCuePoint);
		this.splitCuePoint.Panel1.Controls.Add(this.btnCuePointFileSave);
		this.splitCuePoint.Panel1.Controls.Add(this.tboxCuePointFile);
		this.splitCuePoint.Panel1.Controls.Add(this.btnCuePointDelete);
		this.splitCuePoint.Panel1.Controls.Add(this.btnCuePointFile);
		this.splitCuePoint.Panel1.Controls.Add(this.btnCuePointAdd);
		this.splitCuePoint.Panel2.Controls.Add(this.btnCueParamAdd);
		this.splitCuePoint.Panel2.Controls.Add(this.btnCueParamDelete);
		this.splitCuePoint.Panel2.Controls.Add(this.tblCuePointParams);
		this.splitCuePoint.Size = new System.Drawing.Size(376, 371);
		this.splitCuePoint.SplitterDistance = 180;
		this.splitCuePoint.TabIndex = 10;
		this.tblCuePoint.AllowUserToAddRows = false;
		this.tblCuePoint.AllowUserToDeleteRows = false;
		this.tblCuePoint.AllowUserToResizeRows = false;
		this.tblCuePoint.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tblCuePoint.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
		this.tblCuePoint.ColumnHeadersHeight = 20;
		this.tblCuePoint.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.DisableResizing;
		this.tblCuePoint.Columns.AddRange(this.CuePointName, this.CuePointTime, this.CuePointType);
		this.tblCuePoint.Location = new System.Drawing.Point(0, 33);
		this.tblCuePoint.Margin = new System.Windows.Forms.Padding(1);
		this.tblCuePoint.Name = "tblCuePoint";
		this.tblCuePoint.ReadOnly = true;
		this.tblCuePoint.Size = new System.Drawing.Size(376, 146);
		this.tblCuePoint.TabIndex = 4;
		this.CuePointName.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
		this.CuePointName.HeaderText = "Cue Point Name";
		this.CuePointName.Name = "CuePointName";
		this.CuePointName.ReadOnly = true;
		this.CuePointTime.HeaderText = "Time";
		this.CuePointTime.Name = "CuePointTime";
		this.CuePointTime.ReadOnly = true;
		this.CuePointType.HeaderText = "Type";
		this.CuePointType.Name = "CuePointType";
		this.CuePointType.ReadOnly = true;
		this.CuePointType.Resizable = System.Windows.Forms.DataGridViewTriState.True;
		this.CuePointType.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Automatic;
		this.btnCuePointFileSave.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btnCuePointFileSave.Location = new System.Drawing.Point(278, 5);
		this.btnCuePointFileSave.Name = "btnCuePointFileSave";
		this.btnCuePointFileSave.Size = new System.Drawing.Size(28, 23);
		this.btnCuePointFileSave.TabIndex = 7;
		this.btnCuePointFileSave.Text = "S";
		this.btnCuePointFileSave.UseVisualStyleBackColor = true;
		this.tboxCuePointFile.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tboxCuePointFile.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.tboxCuePointFile.Location = new System.Drawing.Point(3, 7);
		this.tboxCuePointFile.Name = "tboxCuePointFile";
		this.tboxCuePointFile.Size = new System.Drawing.Size(238, 20);
		this.tboxCuePointFile.TabIndex = 2;
		this.tboxCuePointFile.Text = "*.txt";
		this.btnCuePointDelete.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btnCuePointDelete.Location = new System.Drawing.Point(347, 5);
		this.btnCuePointDelete.Name = "btnCuePointDelete";
		this.btnCuePointDelete.Size = new System.Drawing.Size(26, 23);
		this.btnCuePointDelete.TabIndex = 6;
		this.btnCuePointDelete.Text = "-";
		this.btnCuePointDelete.UseVisualStyleBackColor = true;
		this.btnCuePointFile.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btnCuePointFile.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.btnCuePointFile.Location = new System.Drawing.Point(247, 5);
		this.btnCuePointFile.Name = "btnCuePointFile";
		this.btnCuePointFile.Size = new System.Drawing.Size(27, 23);
		this.btnCuePointFile.TabIndex = 3;
		this.btnCuePointFile.Text = "...";
		this.btnCuePointFile.UseVisualStyleBackColor = true;
		this.btnCuePointFile.Click += new System.EventHandler(btnCuePointFile_Click);
		this.btnCuePointAdd.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btnCuePointAdd.Location = new System.Drawing.Point(318, 5);
		this.btnCuePointAdd.Name = "btnCuePointAdd";
		this.btnCuePointAdd.Size = new System.Drawing.Size(26, 23);
		this.btnCuePointAdd.TabIndex = 5;
		this.btnCuePointAdd.Text = "+";
		this.btnCuePointAdd.UseVisualStyleBackColor = true;
		this.btnCueParamDelete.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btnCueParamDelete.Location = new System.Drawing.Point(348, 3);
		this.btnCueParamDelete.Name = "btnCueParamDelete";
		this.btnCueParamDelete.Size = new System.Drawing.Size(25, 23);
		this.btnCueParamDelete.TabIndex = 9;
		this.btnCueParamDelete.Text = "-";
		this.btnCueParamDelete.UseVisualStyleBackColor = true;
		base.AutoScaleDimensions = new System.Drawing.SizeF(6f, 13f);
		base.Controls.Add(this.splitCuePoint);
		base.Name = "CuePointSubPanel";
		base.Size = new System.Drawing.Size(376, 371);
		((System.ComponentModel.ISupportInitialize)this.tblCuePointParams).EndInit();
		this.splitCuePoint.Panel1.ResumeLayout(false);
		this.splitCuePoint.Panel1.PerformLayout();
		this.splitCuePoint.Panel2.ResumeLayout(false);
		this.splitCuePoint.ResumeLayout(false);
		((System.ComponentModel.ISupportInitialize)this.tblCuePoint).EndInit();
		base.ResumeLayout(false);
	}
}
